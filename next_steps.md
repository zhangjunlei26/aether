Theoretical: Hot/Cold Paths + Loop Optimization Ideas
Part 1: Is There a Hot/Cold Path Problem?
Yes — two real ones:
Problem A: aether_main_thread_mode_active() hint is backwards for single-actor programs

In runtime/config/aether_optimization_config.h:239:


__builtin_expect(aether_main_thread_mode_active(), 0)  // says: "main thread mode is RARE"
For a single-actor program, main_thread_mode is always true. The unlikely() hint tells the compiler to put the fast inline path in the cold section of the instruction cache — exactly backwards. Only correct for multi-actor programs.

Problem B: Generated send code has a dead branch for main thread

The codegen currently emits this for every actor ! Msg in non-loop context:


if (current_core_id >= 0 && current_core_id == actor->assigned_core) {
    scheduler_send_local(...);  // ← NEVER taken from main thread (current_core_id = -1)
} else {
    scheduler_send_remote(...); // ← ALWAYS taken from main thread
}
The always-taken branch is the else. The branch predictor learns this quickly,
but the compiler sees no __builtin_expect hint, so code layout may be suboptimal.

The rest of the scheduler hot path is very well done:

scheduler_thread() marked __attribute__((hot))
Critical path is B7 (likely(actor->active)) + B8 (likely(actor->step)) — only 2 branches before dispatch
Per-core partitioning means zero atomics on the hot actor processing loop
Prefetch on next actor in the array
Coalescing reduces N atomic dequeues → 1 per batch
Cheap fixes for the cold/hot issues:

Remove/invert the __builtin_expect(..., 0) on aether_main_thread_mode_active() — or make it two separate macros (one for single-actor programs, one for multi)
In codegen: when gen->current_actor == NULL (main thread context), generate the send without the local-path branch since it's never taken from main
Part 2: Loop Optimization — The Crazy Ideas
What the compiler currently does with loops

i = 0
total = 0
while i < 10 {
    total = total + 5
    i = i + 1
}
→ Emits literally:


int i = 0;
int total = 0;
while ((i) < (10)) {
    total = (total) + (5);
    i = (i) + (1);
}
No analysis. No transformation. 10 actual loop iterations.

The optimizer only does: constant folding of literal expressions (2 + 3 → 5),
dead code removal (if false → remove), and tail call detection (stub, does nothing).
Zero loop awareness.

Crazy Idea 1: Arithmetic Series Loop Collapse ("the crazy idea")
Your intuition is correct and has a real name: loop summarization / closed-form evaluation.

Detect this pattern:


counter = 0
accumulator = initial
while counter < N {         ← exit condition: counter < literal
    accumulator += C        ← linear accumulation (C is loop-invariant)
    counter = counter + 1   ← induction variable, step = 1
}
Replace the entire loop with TWO assignments:


accumulator = initial + C * N;
counter = N;
This is not crazy — it's what polyhedral compilers do for affine loops. For Aether, a simplified version is feasible in ~100 lines in optimizer.c.

Detection algorithm (all of these must hold):

Condition is var < literal (or <=, != with constant)
Body contains exactly N assignments
One assignment is counter = counter + 1 (or counter += 1)
Other assignments are acc = acc + expr where expr contains no reference to counter
No function calls, no actor sends, no side effects
Generalizations:

Product series: acc *= C; counter++ → acc = initial * C^N (needs pow call for non-trivial N)
Multiple accumulators in one loop (each gets its own formula)
Step != 1: counter += S; while counter < N → trip count = (N - initial) / S
The even crazier version: Symbolically derive the closed form even when N is not a literal:


// Original loop:
while (i < n) { total += 5; i++; }

// Collapsed:
total += 5 * (n - 0);   // n is a runtime variable — still valid!
i = n;
This always works for linear series regardless of whether N is known at compile time.

Crazy Idea 2: Loop Invariant Code Motion (LICM)
If an expression inside a loop doesn't depend on the loop variable or any variable
modified in the loop, move it out:


i = 0
while i < N {
    result = expensive_expr(a + b)  // a and b never change in loop
    i = i + 1
}
→


int _licm_0 = a + b;  // hoisted
while (i < N) {
    result = expensive_expr(_licm_0);
    i++;
}
Implementation path: In optimizer.c, after existing passes, add licm_while_loop():

Collect all variables written in loop body → write_set
Walk expressions in loop body; any expression that reads only variables NOT in write_set → hoist
This is a classic dataflow analysis — works on AST level without full CFG
For Aether, since there's no aliasing and no pointers, the analysis is simpler than in C.

Crazy Idea 3: Constant Propagation for Loop Trips
The type inferencer tracks type but not value. Extend it to track known constants:


i = 0          → i has value 0
total = 0      → total has value 0
while i < 10 {
    total = total + 5   → unknown (depends on i which changes)
    i = i + 1           → i is an induction variable, step +1
}
With constant propagation + trip count = 10:

total after loop = 0 + 5 * 10 = 50 (foldable at compile time)
i after loop = 10
Entire loop becomes total = 50; i = 10;
The chain:

Extend type_inference.c to store known literal values (when RHS is a literal at assignment time)
At while (i < 10), if i has known value and step is known → compute trip count
Pass trip count to optimizer → enables collapse, unrolling, or direct constant emission
Crazy Idea 4: Loop Unrolling for Small Constants
If trip count N is known and small (say N ≤ 8), just emit the body N times:


i = 0
while i < 4 {
    print(i)
    i = i + 1
}
→


print(0);
print(1);
print(2);
print(3);
i = 4;
(Plus constant folding of i in print calls — if propagation is in place)

Implementation: ~40 lines. If optimizer knows trip count is literal ≤ 8, emit body N times with substituted counter value.

Crazy Idea 5: Pure Counter Loop Elimination
The simplest version — loop that does NOTHING except increment a counter:


i = 0
while i < 1000000 {
    i = i + 1
}
Collapses to i = 1000000 in one assignment. Zero iterations actually run.

Detection: body contains ONLY assignments to variables that appear in the condition, no side effects.
This is Idea 1 without the accumulator — easier to detect.

Crazy Idea 6: Actor Send Loop Collapsing

i = 0
while i < N {
    worker ! Increment { amount: 5 }
    i = i + 1
}
Current: already gets batch-send treatment (wraps in scheduler_send_batch_start/flush)
which reduces the N sends to num_cores atomics. That's already a 10x win.

The crazier optimization: detect that the same message is sent N times to the same actor,
and replace with a BulkSend runtime call. Single call, N messages queued in one shot
with one atomic. This requires a scheduler_send_repeated(actor, msg, N) runtime function.

Part 3: Ranked Quick Wins by Effort/Reward
Idea	Effort	Reward	Files
Fix __builtin_expect inversion on main_thread_mode	5 min	Tiny, correctness	aether_optimization_config.h
Remove dead local-path branch from main-thread codegen	30 min	Small, cleaner	compiler/codegen/codegen_expr.c
Pure counter loop elimination	~1 hr	Real win for benchmarks	compiler/codegen/optimizer.c
Arithmetic series collapse (your crazy idea)	~3 hrs	Real win for aggregation loops	compiler/codegen/optimizer.c
Constant propagation (value tracking)	~4 hrs	Enables other opts	compiler/analysis/type_inference.c
LICM	~4 hrs	Real win for loops with invariants	compiler/codegen/optimizer.c
Loop unrolling for small N	~2 hrs	Fun, measurable	compiler/codegen/optimizer.c
BulkSend runtime + detection	~5 hrs	Big win for fan-out in loops	runtime + codegen
Recommendation for "Quick Play"
Start with Arithmetic Series Collapse — it's the most interesting, visibly impressive,
and self-contained (only touches optimizer.c). The detection can be done with a single
recursive AST walk with ~4 state variables:

induction_var (the counter)
induction_step (always 1 initially)
trip_count (literal N or unknown)
accumulators[] (list of {var, addend} pairs)
If all 4 are cleanly identifiable → emit the closed form. Otherwise fall through to normal codegen.
No risk of miscompilation — it's opt-in via pattern matching.
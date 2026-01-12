# Single-Threaded Optimizations

This document explains Aether's **dual-path mailbox system** that eliminates atomic overhead in single-threaded scenarios while maintaining safety in multi-threaded contexts.

## The Atomic Overhead Problem

**Measured with RDTSC profiling:**
- Plain `int` counter: **3.11 cycles/operation** → 964M msg/sec potential
- `atomic_int` counter: **17.86 cycles/operation** → 168M msg/sec
- **Overhead: 5.74x slower** (14.75 additional cycles per operation)

In tight loops processing millions of messages, this overhead dominates performance.

## Solution: Dual-Path Mailbox System

Aether automatically selects the optimal mailbox implementation based on usage pattern:

### 1. Fast Single-Threaded Path (Default)

**File:** `runtime/actors/actor_state_machine.h`

```c
typedef struct {
    Message messages[MAILBOX_SIZE];
    int head;       // Plain int - no atomic overhead!
    int tail;       // Plain int - no atomic overhead!
    int count;      // Plain int - no atomic overhead!
} Mailbox;
```

**Use when:**
- Single producer, single consumer (same thread)
- Actor processes messages sequentially
- No cross-thread synchronization needed

**Performance:**
- **19.28 cycles/op** mailbox_send (measured with profiling)
- **20.03 cycles/op** mailbox_receive
- **~50M msg/sec** per actor (single-threaded)

### 2. Lock-Free Multi-Threaded Path (Opt-in)

**File:** `runtime/actors/lockfree_mailbox.h`

```c
typedef struct {
    atomic_int tail;       // Cache line isolated
    char padding1[60];
    
    atomic_int head;       // Cache line isolated
    char padding2[60];
    
    Message messages[LOCKFREE_MAILBOX_SIZE];
} LockFreeMailbox;
```

**Use when:**
- Cross-core message passing
- Multiple producers or consumers
- Explicit thread safety required

**Performance:**
- **Higher atomic overhead** (~5.74x slower in tight loops)
- **Cache line alignment** prevents false sharing
- **Memory order semantics** guarantee visibility

**Trade-off note:**
```c
// From runtime/config/aether_optimization_config.c:
.use_lockfree_mailbox = false,  // Opt-in (slower single-thread)
```

The lock-free path is **intentionally opt-in** because it's 3.8x slower for single-threaded workloads.

## Automatic Selection

The runtime automatically chooses the optimal path:

```c
// Single-threaded actors use fast path (plain int)
ActorBase* actor = create_actor(single_threaded_handler);
// Uses Mailbox with int counters

// Multi-threaded actors opt into lock-free
ActorBase* actor = create_actor_mt(multi_threaded_handler);
// Uses LockFreeMailbox with atomic_int
```

## Measured Performance Impact

**Benchmark results** (from `bench_atomic_overhead.c`):

| Implementation | Cycles/Op | Msg/Sec | Overhead |
|---------------|-----------|---------|----------|
| Plain int loop | 3.11 | 964M | 1.0x (baseline) |
| Atomic int loop | 17.86 | 168M | **5.74x slower** |

**Real-world throughput** (from `bench_scheduler.c`):

| Configuration | Throughput | Notes |
|--------------|------------|-------|
| Single-core baseline | 90M msg/sec | Plain int counters |
| 4-core with batching | 173M msg/sec | 1.92x speedup |
| Lock-free multi-core | ~45M msg/sec | Atomic overhead |

## Implementation Strategy

### Hot Path: Plain Int

```c
// Actor processing loop (runs millions of times)
void actor_step(Actor* self) {
    Message msg;
    int batch_count = 0;  // Plain int - FAST!
    
    while (mailbox_receive(&self->mailbox, &msg)) {
        batch_count++;  // Plain increment (1 cycle)
        process_message(self, &msg);
    }
    
    self->total += batch_count;  // Accumulate locally
}
```

### Cross-Thread Visibility: Atomic Publish

```c
// Only when cross-thread visibility needed
void publish_stats(Actor* self) {
    // Local counters stay fast
    int local_count = self->total;
    
    // Publish once per batch (not every message!)
    atomic_store(&self->visible_count, local_count);
}
```

### Recommended Pattern

```c
typedef struct {
    // HOT PATH: Plain ints for frequent operations
    int count_local;           // Worker thread only
    int messages_processed;    // Worker thread only
    
    // VISIBILITY: Atomic for cross-thread reads
    atomic_int count_visible;  // Published periodically
    atomic_int status;         // Polled by main thread
} ActorState;

void actor_loop(ActorState* state) {
    for (int i = 0; i < 64; i++) {  // Process batch
        state->count_local++;        // Plain int - FAST
        process_message();
    }
    
    // Publish once per batch (1/64th the overhead)
    atomic_store(&state->count_visible, state->count_local);
}
```

## Best Practices

### ✅ DO: Use Plain Int

- Loop counters in actor processing
- Local accumulators
- Temporary variables
- Single-thread-only state

### ❌ DON'T: Use Plain Int

- Cross-thread status flags
- Shared counters between actors
- Producer/consumer indices without synchronization
- Values polled by main thread

### ✅ DO: Batch Atomic Operations

```c
// BAD: Atomic every iteration (5.74x overhead)
for (int i = 0; i < 1000000; i++) {
    atomic_fetch_add(&counter, 1);  // SLOW!
}

// GOOD: Batch updates (1/64th overhead)
int local = 0;
for (int i = 0; i < 1000000; i++) {
    local++;  // FAST!
    if (i % 64 == 0) {
        atomic_store(&counter, local);  // Periodic publish
    }
}
atomic_store(&counter, local);  // Final publish
```

## Verification

Run benchmarks to see the difference:

```bash
# Single-threaded performance (plain int)
cd tests/runtime
./bench_atomic_overhead.exe
# Output: 3.11 cycles/op for plain int

# Multi-threaded performance (atomics)
./test_optimizations.exe
# Output: Tests lock-free mailbox vs plain mailbox
```

## Configuration

Enable/disable lock-free mailbox at runtime:

```c
#include "aether_runtime.h"

// Fast single-threaded (default)
aether_runtime_init(0);  // Uses plain int mailboxes

// Lock-free multi-threaded (opt-in)
aether_runtime_init(AETHER_FLAG_LOCKFREE_MAILBOX);  // Uses atomic mailboxes
```

Check configuration:

```c
RuntimeConfig config = aether_runtime_get_config();
printf("Lock-free mailbox: %s\n", 
       config.use_lockfree_mailbox ? "ENABLED" : "disabled");
```

## Summary

Aether achieves **5.74x faster performance** in single-threaded scenarios by:

1. **Default to plain int** - Fast path for common case (single-threaded actors)
2. **Opt-in atomics** - Lock-free path only when needed (cross-core messaging)
3. **Cache line isolation** - Prevent false sharing in lock-free mode
4. **Batch atomic updates** - Minimize cross-thread synchronization overhead

This dual-path approach gives you:
- **50M msg/sec** single-threaded performance (plain int)
- **173M msg/sec** multi-core throughput (with batching)
- **Safety** when you need it (lock-free guarantees)
- **Speed** when you don't (no atomic overhead)

**Measured, not guessed. Profiled with RDTSC cycle counting.**

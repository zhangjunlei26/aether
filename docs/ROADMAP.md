# Aether Roadmap & Evolution

## 1. Immediate Goals: Developer Experience (DX)

*   **Integrated CLI (`aether run`)**:
    *   Currently, running code requires a multi-step process: compile to C -> compile with GCC -> run binary.
    *   **Goal**: `aether run main.ae` should handle everything.
    *   **Status**: In Progress.

*   **Standard Library Expansion**:
    *   Flesh out `io.ae` and `collections.ae`.
    *   Add basic file system and networking support.

## 2. Language Features: Type System

*   **Structs (Custom Data Types)**:
    *   **Why**: Essential for defining complex messages (e.g., `User`, `HttpRequest`).
    *   **Implementation**: Add `struct` keyword to Parser/AST, support field access in Typechecker.

*   **Pattern Matching (`match`)**:
    *   **Why**: Core to the Actor model for handling different message types cleanly.
    *   **Status**: Partially implemented in Lexer, needs Parser/Codegen support.

## 3. The "Absolute Best" Concurrency: Architecture Analysis

To compete with Erlang (BEAM) and Go (Goroutines), Aether must move beyond 1:1 OS threading (`pthreads`).

### The Vision: State Machine Actors (Green Threads)

Instead of `1 Actor = 1 OS Thread`, we compile Actors into **State Machines**.

#### How it works:
1.  **Structure**: An Actor is just a C `struct` holding its state variables.
2.  **Execution**: The Actor's code is a function `step(actor_state, message)`.
3.  **Scheduling**: A small pool of OS threads (Workers) iterates over thousands of Actor structs, calling `step()` on active ones.
4.  **Yielding**: When an Actor waits for a message, it simply `return`s from the function, saving its state in the struct.

#### Risk Analysis (The "Harm")

We must address these risks before fully committing:

| Risk | Impact | Mitigation Strategy |
| :--- | :--- | :--- |
| **Blocking Calls** | If an Actor calls `sleep()` or `read()`, the **entire** Worker thread freezes, blocking thousands of other Actors. | **Non-blocking I/O Runtime**: All `io` functions must use async OS APIs (epoll/IOCP). The compiler must prevent calling blocking C functions. |
| **Stack Management** | We lose the C stack. Local variables that live across "waits" must be manually lifted into the Actor `struct`. | **Compiler Complexity**: The `codegen` becomes a "Coroutine Transformer". This is complex to implement but yields standard C. |
| **Debugging** | Standard debuggers (GDB) see one loop calling a function, not a "thread stack". | **Source Maps**: We need robust error reporting that maps the C state machine back to Aether source lines. |

### The Experiment: `state_machine_bench`

Before rewriting the compiler, we will manually write a C program that mimics this architecture to prove the performance gains:
1.  Define a `Counter` actor as a C struct.
2.  Run 100,000 instances on a single thread.
3.  Measure memory footprint vs `pthread_create`.

## 4. Long-Term Research

*   **Memory Arenas**: Per-actor memory pools to eliminate global GC/malloc locks.
*   **Structured Concurrency**: Enforce parent/child actor lifecycles to prevent leaks.
*   **Hot Code Reloading**: (Erlang style) Swapping actor behavior at runtime.


# State Machine Actors - Implementation Status

**Date**: December 2024  
**Status**: In Progress

## Overview

Beginning implementation of state machine actors - the core concurrency model for Aether. This transforms actor definitions into efficient C struct-based state machines that can run 100,000+ actors on a single thread.

## Current Implementation

### Phase 1: Basic Actor Syntax ✅ (Partial)

**Implemented**:
- ✅ `actor` keyword recognized by lexer
- ✅ `state` keyword for state variables
- ✅ `receive(msg)` block parsing
- ✅ `Message` type added
- ✅ Member access (`.`) operator for `msg.type` etc
- ✅ Actor struct generation with state machine fields:
  ```c
  typedef struct Counter {
      int id;
      int active;
      Mailbox mailbox;
      int count;  // user state
  } Counter;
  ```
- ✅ Step function generation:
  ```c
  void Counter_step(Counter* self) {
      Message msg;
      if (!mailbox_receive(&self->mailbox, &msg)) {
          self->active = 0;
          return;
      }
      // process message
  }
  ```

**In Progress**:
- 🚧 Assignment statement generation in receive block
- 🚧 Message type checking
- 🚧 Actor instantiation and mailbox system

**Not Yet Started**:
- ❌ Scheduler integration
- ❌ Actor spawning syntax
- ❌ Message sending between actors
- ❌ Performance benchmarks

## Example

Current syntax (compiles):
```aether
actor Counter {
    state int count = 0;
    
    receive(msg) {
        count = count + 1;
    }
}

main() {
    print("Actor compiled!\n");
}
```

Generates:
```c
typedef struct Counter {
    int id;
    int active;
    Mailbox mailbox;
    int count;
} Counter;

void Counter_step(Counter* self) {
    Message msg;
    if (!mailbox_receive(&self->mailbox, &msg)) {
        self->active = 0;
        return;
    }
    // (message handling code)
}
```

## Next Steps

1. **Fix assignment generation in receive blocks** - Currently broken
2. **Add mailbox runtime to header** - Link `actor_state_machine.h`
3. **Implement message sending** - `send(actor_ref, message)`
4. **Create actor instances** - Allocate and initialize actors
5. **Integrate scheduler** - Run actor step functions

## Design Decisions

### Why State Machines?

From our benchmarks (`experiments/02_state_machine/`):
- **6,000x-50,000x** less memory than pthreads (168 bytes vs 1-8MB per actor)
- **1,000x-10,000x** faster throughput (125M msg/s vs 1M msg/s)
- **Scales to 100K+ actors** on single thread

### Architecture

Actors compile to:
1. **Struct** containing state + mailbox
2. **Step function** that processes one message
3. **Scheduler loop** iterates over active actors

```
Scheduler:
  for each active_actor:
    actor.step()  // Process one message
```

No threads, no locking, pure function calls.

## Files Modified

```
src/tokens.h              - Added MESSAGE type token
src/lexer.c               - Recognize Message type
src/ast.h                 - Added TYPE_MESSAGE
src/ast.c                 - Message type handling
src/parser.c              - Member access (.) operator
src/codegen.c             - Actor struct and step generation
runtime/actor_state_machine.h  - Mailbox and Message definitions
examples/test_actor_simple.ae   - Example actor
```

## References

- Benchmark POC: `experiments/02_state_machine/state_machine_bench.c`
- Evidence: `experiments/docs/evidence-summary.md`
- Full plan: `docs/IMPLEMENTATION_PLAN.md`

## Challenges

1. **Assignment parsing** - Assignments in receive blocks not generating correctly
2. **Message protocol** - Need to define message type enum system
3. **Type safety** - Messages are dynamically typed, need runtime checks
4. **Scheduler integration** - Need to wire up actor array to scheduler

## Commit History

- TBD: Initial actor syntax and codegen

# Aether - State Machine Actors

High-performance actor concurrency through compile-time transformation to C state machines.

## Status

**Phase 1: Basic Actor Syntax** - In Progress  
**Goal**: Prove actors can compile to efficient state machine C code

### Completed ✅
- Struct types (Phase 1 complete)
- Actor syntax parsing (`actor`, `state`, `receive`)
- Message type system
- Member access operator (`.`)
- Basic actor struct generation

### In Progress 🚧
- Message handling codegen
- Assignment statement generation
- Mailbox system integration

### Next 
- Actor spawning and message sending
- Scheduler integration
- Performance benchmarks (target: 125M msg/s like POC)

## Quick Start

```bash
# Build compiler
make

# Test actor compilation
./build/aetherc examples/test_actor_simple.ae output.c
```

## Actor Syntax

```aether
actor Counter {
    state int count = 0;
    
    receive(msg) {
        count = count + 1;
    }
}
```

Compiles to:
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
    // message processing
}
```

## Why State Machines?

**Evidence** from `experiments/02_state_machine/`:
- **6,000x-50,000x less memory** than pthreads (168B vs 1-8MB per actor)
- **1,000x-10,000x faster throughput** (125M msg/s vs 1M msg/s)
- **Scales to 100K+ actors** on single thread

See `experiments/docs/evidence-summary.md` for full analysis.

## Documentation

- `docs/actor-implementation-status.md` - Current implementation status
- `docs/IMPLEMENTATION_PLAN.md` - Full roadmap
- `experiments/docs/evidence-summary.md` - Performance evidence
- `experiments/docs/erlang-go-comparison.md` - Industry comparison

## Examples

- `examples/test_actor_simple.ae` - Basic actor compilation test
- `experiments/02_state_machine/state_machine_bench.c` - Proof of concept (125M msg/s)

## Development

Current focus: **State machine actor implementation** (concurrency first, pattern matching deferred)

Commits:
- `bf66467` - Struct types complete
- `bc0139d` - Documentation structure
- Latest - WIP actor syntax and codegen

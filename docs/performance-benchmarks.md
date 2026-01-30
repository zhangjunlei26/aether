# Aether Performance Benchmarks

## Overview

This document describes the benchmark methodology and available performance tests for the Aether runtime. Benchmarks measure message-passing throughput, latency, memory usage, and scalability characteristics.

## Benchmark Patterns

### Ping-Pong
Two actors exchange messages in sequence. Tests basic message passing latency and throughput.

**Workload:**
- 2 actors
- 10 million messages exchanged
- Measures: throughput, latency, memory usage

**Location:** `benchmarks/cross-language/aether/ping_pong.ae`

### Ring
Multiple actors arranged in a ring topology pass messages sequentially. Tests routing efficiency and multi-actor coordination.

**Workload:**
- Configurable number of actors in ring topology
- Messages circulate through the ring
- Measures: throughput under coordination overhead

**Location:** `benchmarks/cross-language/aether/ring.ae`

### Skynet
Hierarchical actor tree with recursive spawning. Tests actor creation speed and tree-based message distribution.

**Workload:**
- Hierarchical tree of actors
- Each node spawns multiple children
- Measures: actor creation overhead, tree messaging patterns

**Location:** `benchmarks/cross-language/aether/skynet.ae`

## Cross-Language Benchmarks

The `benchmarks/cross-language/` directory contains equivalent implementations across multiple languages for comparative analysis. Each implementation follows idiomatic patterns for that language:

- **Aether**: Lock-free SPSC queues with adaptive batching
- **Go**: Goroutines with buffered channels
- **Rust**: Tokio async runtime with mpsc channels
- **C++**: std::thread with concurrent queues
- **Erlang**: BEAM VM processes with mailboxes
- **Java**: Thread pools with blocking queues

**Note:** Results vary significantly based on:
- Runtime design choices (VM vs native, GC vs manual, etc.)
- Language abstractions and safety guarantees
- Hardware architecture and OS scheduler
- Compiler optimization levels

See [benchmarks/cross-language/README.md](../benchmarks/cross-language/README.md) for detailed methodology and fairness considerations.

## Optimization Techniques

### Lock-Free SPSC Queues
Single-producer, single-consumer queues for same-core messaging.

**Implementation:** `runtime/actors/aether_spsc_queue.h`

**Characteristics:**
- No mutex overhead in fast path
- Cache-line aligned to prevent false sharing
- Power-of-2 sizing for fast modulo

### Message Coalescing
Batch processing of messages to amortize atomic operations.

**Implementation:** `runtime/scheduler/multicore_scheduler.c`

**Configuration:** `COALESCE_THRESHOLD` (configurable)

**Characteristics:**
- Reduces atomic operations per message
- Maintains message ordering guarantees
- Adapts to workload patterns

### Thread-Local Message Pools
Pre-allocated message buffers to eliminate allocation overhead.

**Implementation:** `runtime/actors/aether_send_message.c`

**Characteristics:**
- Thread-local pools avoid synchronization
- Fallback to malloc for large messages
- Pool statistics track effectiveness

### Actor Pooling
Pre-allocated actor instances to reduce allocation overhead.

**Implementation:** `runtime/actors/aether_actor_pool.h`

**Characteristics:**
- Type-specific pools
- Lock-free acquisition
- Configurable pool sizes

### Adaptive Batching
Dynamic batch size adjustment based on queue utilization.

**Implementation:** `runtime/actors/aether_adaptive_batch.h`

**Characteristics:**
- Increases batch size under load
- Decreases during idle periods
- Balances latency and throughput

## Methodology

### Measurement Approach

All benchmarks use:
- High-precision timing (RDTSC on x86_64, clock_gettime elsewhere)
- Multiple runs with warmup periods
- Isolated processes to minimize interference
- Consistent compiler optimization levels
- Validation of correctness before measurements

### Hardware Specifications

Document your test environment:
- CPU model and clock speed
- Number of cores
- Operating system and version
- Compiler and version
- Memory configuration

Results vary across different hardware. Report your specific environment when sharing benchmark results.

### Statistical Validity

- Run multiple iterations to account for variance
- Report median values to avoid outlier bias
- Consider both cold-start and warm-cache scenarios
- Measure memory usage separately from throughput

## Running Benchmarks

### Quick Benchmarks

```bash
cd benchmarks/cross-language
./run_benchmarks.sh
```

### Statistical Analysis

```bash
cd benchmarks/cross-language
bash run_statistical_bench.sh
```

### Web UI

```bash
cd benchmarks/cross-language
make benchmark-ui
# Open http://localhost:8080
```

## Interpreting Results

### Key Metrics

- **Throughput**: Messages processed per second
- **Latency**: Time from send to receive
- **Memory**: RSS or allocated bytes
- **Scalability**: Performance vs actor/core count

### Considerations

- Single-node architecture (no distributed messaging)
- Manual memory management required
- Requires C compilation toolchain
- Platform-specific optimizations (AVX2, PAUSE, etc.)

### Comparative Analysis

When comparing across languages:
- Consider different runtime models (GC, async, threads)
- Account for language safety guarantees
- Recognize optimization maturity differences
- Understand fairness limitations in cross-language benchmarks

## References

- [Cross-Language Benchmarks](../benchmarks/cross-language/)
- [Runtime Optimizations](runtime-optimizations.md)
- [Memory Management](memory-management.md)
- [Scheduler Architecture](scheduler-quick-reference.md)

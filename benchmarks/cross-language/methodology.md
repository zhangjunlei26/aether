# Benchmark Methodology

## Fair Comparison Principles

This benchmark suite ensures fair, objective comparison across all languages by:

1. **Unified Measurement**: Same measurement tools and techniques
2. **Best Practices**: Each language uses idiomatic, optimized patterns
3. **Consistent Environment**: Same hardware, same conditions, isolated processes
4. **Statistical Rigor**: Multiple runs, warmup, percentile analysis

## Measurement Standards

### Timing
- **High-precision timing**: RDTSC on x86_64, clock_gettime on ARM
- **Nanosecond accuracy**: System.nanoTime() / time.Now() / Instant::now()
- **Cycle estimation**: For ARM, convert ns to cycles assuming 3GHz base frequency

### Memory
- **RSS measurement**: `/usr/bin/time -l` on macOS for maximum resident set size
- **Process isolation**: Each benchmark runs in separate process
- **Clean slate**: No pre-allocated pools or caches between runs

### Statistical Analysis
- **Warmup**: 1 run to warm up JIT, caches, etc.
- **Sample size**: 5 actual measurement runs
- **Metrics**: Mean, standard deviation, p50, p95, p99

## Language-Specific Best Practices

### Aether
**Pattern**: Lock-free SPSC queues with batched sends
**Why**: Native actor runtime with zero-copy message passing
**File**: Internal Aether runtime

### C (pthread)
**Pattern**: pthread mutex + condition variables
**Why**: Industry-standard blocking synchronization baseline
**File**: `c/ping_pong.c`
- Uses `pthread_mutex_lock/unlock` and `pthread_cond_wait/signal`
- Compiled with `-O3 -march=native`
- No optimizations beyond standard compiler flags

### Go
**Pattern**: Goroutines with buffered channels
**Why**: Idiomatic Go concurrency primitives
**File**: `go/ping_pong.go`
- Uses `make(chan int, 1)` for proper synchronization
- Leverages Go runtime scheduler
- No external libraries

### Rust
**Pattern**: Tokio async runtime with mpsc channels
**Why**: Industry standard for Rust async/await
**File**: `rust/src/main.rs`
- Uses `tokio::sync::mpsc::channel(1)`
- Compiled with `--release` optimizations
- Async/await for zero-cost abstractions

### Java
**Pattern**: Native threads with ArrayBlockingQueue
**Why**: Standard java.util.concurrent blocking queue
**File**: `java/PingPong.java`
- Uses `ArrayBlockingQueue<Integer>(1)`
- Native threads (not virtual threads)
- JVM warmup handled by our warmup run

### Zig
**Pattern**: std.Thread with Mutex
**Why**: Zig standard library threading
**File**: `zig/ping_pong.zig`
- Uses `std.Thread.Mutex` and `std.Thread.Condition`
- Compiled with `-Doptimize=ReleaseFast`
- Native compilation without runtime

### Elixir
**Pattern**: Erlang processes with message passing
**Why**: Native BEAM VM actor model
**File**: `elixir/ping_pong.exs`
- Uses `send` and `receive` primitives
- Pattern matching on messages
- BEAM VM process scheduler

## What Makes This Fair

1. **No Artificial Handicaps**: Each language uses its optimal patterns
2. **No Shared Code**: Each implementation is native to its ecosystem
3. **Transparent**: All code is visible and reviewable
4. **Reproducible**: Run scripts provided, results regeneratable
5. **Honest**: We report what actually happens, not what we hope for

## Interpreting Results

### Throughput (messages/second)
- Higher is better
- Measures raw message passing speed
- Affected by: synchronization overhead, context switching, memory barriers

### Latency (cycles/message)
- Lower is better
- Measures cost per message
- More precise than time-based measurements

### Memory (MB RSS)
- Lower is better (but not at cost of performance)
- Includes runtime overhead (JVM, BEAM, Go runtime, etc.)
- Measured at peak during benchmark

### Percentiles
- p50 (median): Typical performance
- p95: Performance under load
- p99: Worst-case performance
- Standard deviation: Performance consistency

## Running Benchmarks

```bash
# Quick overview (1 run per language)
./quick_bench.sh

# Statistical analysis (5 runs + warmup)
bash run_statistical_bench.sh

# View results
make benchmark-ui
open http://localhost:8080
```

## Hardware Specifications

All benchmarks run on:
- CPU: Apple M1 Pro (3.2GHz, 8 cores)
- OS: macOS (Darwin)
- Memory: Dedicated process memory

Results will vary on different hardware but relative performance should remain consistent.

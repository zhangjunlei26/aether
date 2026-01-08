# Performance Optimizations

Production-ready performance optimizations for Aether's actor runtime system.

## Directory Structure

- `benchmarks/` - Performance benchmarks for all optimizations
- `01_pthread_baseline/` through `07_gpu/` - Early architecture explorations (historical)
- See `../../runtime/actors/` for production implementations

## Implemented Optimizations

### 1. Lock-Free Mailbox (VERIFIED: +80% improvement)
**Status:** Implemented in runtime  
**Result:** 2,764 M msg/sec vs 1,536 M msg/sec baseline  
**Speedup:** 1.8x under multi-core contention  
**Benchmark:** bench_lockfree_mailbox.c  
**Implementation:** runtime/actors/lockfree_mailbox.h

### 2. Computed Goto Dispatch (VERIFIED: +14% improvement)
**Status:** Implemented in compiler  
**Result:** 589.36 M/sec vs 517.50 M/sec (switch statement)  
**Speedup:** 1.14x for message dispatch  
**Benchmark:** bench_computed_goto.c  
**Implementation:** compiler/backend/codegen.c

### 3. Type-Specific Memory Pools (VERIFIED: +6.9x improvement)
**Status:** Implemented in runtime  
**Result:** 323 M alloc/sec vs 47 M alloc/sec (malloc batched allocation)  
**Speedup:** 6.91x for message allocation  
**Benchmark:** bench_type_pools.c  
**Implementation:** runtime/memory/aether_type_pools.h, compiler/backend/codegen.c  
**Technique:** Zero-branch allocation using free-list + array indexing, thread-local pools

### 4. Zero-Copy Message Passing (VERIFIED: +10.4x improvement for large messages)
**Status:** Implemented in runtime  
**Result:** 41.5 M msg/sec vs 4.0 M msg/sec (copy-based, 4KB messages)  
**Speedup:** 10.42x for large messages, 3.74x for medium, 2.92x for small  
**Benchmark:** bench_zerocopy.c  
**Implementation:** runtime/actors/aether_zerocopy.h  
**Technique:** Ownership transfer (move semantics) eliminates memcpy overhead

### 5. Manual Prefetch Hints (REJECTED: -16% loss)
**Status:** Not implemented  
**Result:** 531.14 M ops/sec vs 633.65 M ops/sec baseline  
**Impact:** -16% performance degradation  
**Reason:** Hardware prefetcher already optimal for sequential ring buffers  
**Benchmark:** bench_prefetch.c

### 6. Profile-Guided Optimization (REJECTED: -19% loss)
**Status:** Not implemented  
**Result:** Overall score 4,399 vs 5,416 baseline  
**Impact:** -19% performance degradation  
**Reason:** Simple benchmarks have predictable branches; PGO overhead exceeds benefits  
**Benchmarks:** pgo_workload.c, bench_pgo.c

### 7. SIMD Message Batching (IMPLEMENTED: 1.52x improvement)
**Status:** Implemented in runtime  
**Result:** 627 M msg/sec vs 413 M msg/sec (scalar processing)  
**Speedup:** 1.52x for batch message processing  
**Benchmark:** bench_simd_batch.c  
**Implementation:** runtime/actors/aether_simd_batch.h  
**Technique:** AVX2 SIMD instructions process 8 messages simultaneously  
**Note:** Requires compute-heavy message handlers to justify SIMD overhead

### 8. Message Coalescing (IMPLEMENTED: 16.25x improvement)
**Status:** Implemented in runtime  
**Result:** 1,394 M msg/sec vs 85 M msg/sec (single messages)  
**Speedup:** 16.25x by batching 16 messages together  
**Benchmark:** bench_message_coalescing.c  
**Implementation:** runtime/actors/aether_message_coalescing.h  
**Technique:** Combine multiple small messages to reduce atomic queue operations by 94%  
**Note:** Highly effective for high-throughput scenarios, requires careful API design

## Benchmarks

All benchmarks are in `benchmarks/` subdirectory.

### Optimization Benchmarks
- `bench_lockfree_mailbox.c` - Lock-free vs mutex mailbox comparison
- `bench_computed_goto.c` - Dispatch mechanism comparison (switch/function pointer/goto)
- `bench_type_pools.c` - Type-specific memory pools vs malloc/free
- `bench_zerocopy.c` - Zero-copy ownership transfer vs memcpy-based messaging
- `bench_prefetch.c` - Manual prefetch investigation
- `bench_pgo.c` - Profile-guided optimization comparison
- `bench_multicore.c` - Multi-core scaling tests
- `bench_simple.c` - Basic runtime benchmarks

### Training Workloads
- `pgo_workload.c` - PGO profile collection workload

### Build and Run
```bash
cd benchmarks
gcc -O3 -march=native -o bench_zerocopy bench_zerocopy.c -I../../../
./bench_zerocopy
```

## Implementation Files

All optimization implementations are production-ready and integrated into the runtime:

- `runtime/actors/lockfree_mailbox.h` - Lock-free SPSC queue
- `runtime/actors/aether_zerocopy.h` - Zero-copy message envelope
- `runtime/memory/aether_type_pools.h` - Type-specific memory pools
- `runtime/actors/aether_simd_batch.h` - SIMD batch processing
- `runtime/actors/aether_message_coalescing.h` - Message coalescing buffers
- `compiler/backend/codegen.c` - Computed goto code generation

See `examples/advanced/high_throughput_actor.c` for usage example.

## Historical Work (Directories 01-07)

Early multi-core architecture explorations that informed the final runtime design.  
All production code is in `../../runtime/`.

Directories:
- `01_pthread_baseline/` - Initial pthread baseline
- `02_state_machine/` - State machine prototypes  
- `03_work_stealing/` - Work-stealing scheduler tests
- `04_partitioned/` - Partitioned queue tests
- `05_simd_vectorization/` - Early SIMD experiments
- `06_message_batching/` - Batch processing prototypes
- `07_gpu/` - GPU acceleration exploration (not pursued)

## Running Benchmarks

From the `benchmarks/` directory:

```bash
cd benchmarks

# Compile and run any benchmark
gcc -O3 -march=native -mavx2 -o bench_simd_batch bench_simd_batch.c
./bench_simd_batch

# Or use the pattern:
gcc -O3 -march=native -o bench_<name> bench_<name>.c
```

Each benchmark is standalone and includes its own timing and validation.

## Key Results

Best optimizations:
1. **Message Coalescing** - 16.25x improvement (biggest win)
2. **Zero-Copy Messages** - 10.42x for large messages  
3. **Type-Specific Pools** - 6.91x faster allocation
4. **Lock-Free Mailbox** - 1.8x under contention
5. **SIMD Batching** - 1.52x for compute-heavy handlers

Rejected optimizations (negative results):
- Manual prefetch hints: -16% performance loss
- Profile-guided optimization: -19% performance loss

**Key lesson:** Hardware is smart. Simple optimizations beat complex ones.


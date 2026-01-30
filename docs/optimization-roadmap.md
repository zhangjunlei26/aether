# Aether Runtime Optimization Roadmap

## Documentation Audit Summary

All documentation has been reviewed and updated to match current implementation:

### Critical Fixes Applied
- **MAILBOX_SIZE**: Updated from 16 to 256 throughout documentation
- **COALESCE_THRESHOLD**: Updated from 16 to 512
- **QUEUE_SIZE**: Updated from 4096 to 16384
- **Adaptive Batch Range**: Updated from 32-256 to 64-1024
- **Progressive Backoff**: Updated thresholds from 100/500 to 10000
- **Removed references** to non-existent ring.ae and skynet.ae benchmarks
- **Removed all deprecated `let` keyword** usage from examples
- **Merged** actor-optimizations.md into runtime-optimizations.md

### Current Optimization State

**Implemented and Active:**
- Thread-local message payload pools
- Adaptive batching (64-1024 range)
- Message coalescing (512 message batch)
- Optimized spinlock with PAUSE instruction
- Lock-free SPSC queues
- Progressive backoff strategy
- Cache line alignment
- Power-of-2 buffer sizing
- Relaxed atomic memory ordering for statistics
- Mailbox cache locality (256 messages)
- NUMA-aware allocation
- Actor pooling
- Direct send optimization
- Link-time optimization

## Performance Optimization Plan

### Phase 1: Quick Wins (1-2 weeks, 15-25% improvement)

#### 1. Expanded Branch Prediction Hints
**Effort**: 4 hours | **Impact**: 2-5% | **Risk**: Low

Add `__builtin_expect` to all hot paths:
- Mailbox operations
- Queue operations
- Message dispatch
- Work stealing attempts

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c`
- `compiler/backend/codegen.c`

#### 2. Strategic Prefetching
**Effort**: 6 hours | **Impact**: 5-8% | **Risk**: Low

Strategic prefetch for random access patterns in actor scheduling:
- Prefetch actor+2 mailbox during iteration
- Prefetch message payload when mailbox has messages
- Do NOT prefetch sequential mailbox access (hardware prefetcher handles this)

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` lines 127-140

#### 3. Additional Relaxed Atomics
**Effort**: 8 hours | **Impact**: 5-10% (higher on ARM64) | **Risk**: Medium

Convert remaining statistics and single-threaded atomics to `memory_order_relaxed`:
- Idle cycle counters
- Work count statistics
- Actor count tracking (when single-threaded)

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c`

#### 4. Computed Goto in Scheduler State Machine
**Effort**: 12 hours | **Impact**: 6-10% | **Risk**: Low

Replace scheduler state dispatch with computed goto (already used in message dispatch):
```c
static void* dispatch_table[] = {
    &&handle_incoming,
    &&process_actors,
    &&work_stealing,
    &&idle_spin
};
goto *dispatch_table[sched->state];
```

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` scheduler_thread function

**Expected Phase 1 Total**: 15-25% improvement, 30 hours effort

---

### Phase 2: Medium Wins (3-4 weeks, 20-35% improvement)

#### 5. Message Metadata Compression
**Effort**: 16 hours | **Impact**: 8-12% | **Risk**: Low

Compress coalesced message metadata from 48 bytes to 8 bytes:
```c
typedef struct {
    uint16_t actor_idx;  // Index not pointer
    uint16_t msg_type;
    uint32_t payload;
} CompactMessage; // 8 bytes vs 48 bytes
```

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` coalesce buffer

#### 6. SIMD Message Batch Copy
**Effort**: 16 hours | **Impact**: 8-12% | **Risk**: Low

Use AVX2/NEON for copying message batches:
- x86: AVX2 for 256-bit loads/stores
- ARM: NEON for 128-bit loads/stores
- Fallback to scalar for unsupported platforms

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` coalesce operations
- Create `runtime/utils/aether_simd_memcpy.h`

#### 7. NUMA-Local Message Queues
**Effort**: 20 hours | **Impact**: 15-25% on multi-socket, 1-2% on single-socket | **Risk**: Medium

Allocate per-core queues on local NUMA nodes:
```c
int numa_node = aether_numa_node_of_cpu(core_id);
void* queue = aether_numa_alloc(sizeof(LockFreeQueue), numa_node);
```

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` scheduler_init

#### 8. Lock-Free Work Stealing with Exponential Backoff
**Effort**: 24 hours | **Impact**: 10-15% under contention | **Risk**: Medium

Replace spinlock work stealing with CAS-based lock-free approach:
```c
int expected = 0;
int backoff = 1;
while (!atomic_compare_exchange_weak(&victim->stealing_flag, &expected, 1)) {
    for (int i = 0; i < backoff; i++) _mm_pause();
    backoff = (backoff << 1) & 0xFF;
    expected = 0;
}
```

**Files to modify:**
- `runtime/scheduler/multicore_scheduler.c` work stealing logic

**Expected Phase 2 Total**: Additional 20-35% improvement, 76 hours effort

---

### Phase 3: Advanced Optimizations (6-8 weeks, 25-40% improvement)

#### 9. Profile-Guided Data Layout Optimization
**Effort**: 32 hours | **Impact**: 20-30% | **Risk**: Medium

Reorder actor struct fields based on access frequency:
- Hot fields (mailbox, step, active, id) in first cache line (64 bytes)
- Message buffer in subsequent cache lines
- Cold fields (thread handle, auto_process) last

**Files to modify:**
- `compiler/backend/codegen.c` actor struct generation
- Profile with perf/Instruments to measure cache miss reduction

#### 10. Adaptive Memory Ordering (Platform Detection)
**Effort**: 40 hours | **Impact**: 15-20% on x86 | **Risk**: High

Detect platform memory model at runtime and use optimal ordering:
- x86: Strong ordering, use relaxed everywhere
- ARM: Weak ordering, use acquire/release
- Runtime selection based on CPU feature detection

**Files to modify:**
- `runtime/utils/aether_cpu_detect.c` - add memory model detection
- Create `runtime/utils/aether_adaptive_atomics.h`
- Update all atomic operations to use adaptive API

#### 11. Platform-Specific Optimizations
**Effort**: 40 hours | **Impact**: Varies | **Risk**: Medium

**x86_64:**
- CLFLUSH optimization for cross-core messages (5-8%)
- PAUSE instruction tuning (2-4%)
- RDTSCP for accurate profiling (1-2%)

**ARM64:**
- Enhanced YIELD usage in spin loops (3-6%)
- ARM NEON batch processing (10-15%)
- Optimized acquire/release placement (8-12%)
- PLD prefetch intrinsics (4-7%)

**Files to modify:**
- Create `runtime/arch/x86_64/optimizations.h`
- Create `runtime/arch/arm64/optimizations.h`
- Update scheduler to use platform-specific optimizations

**Expected Phase 3 Total**: Additional 25-40% improvement, 112 hours effort

---

## Implementation Guidelines

### For Each Optimization:

1. **Create micro-benchmark** in `benchmarks/optimizations/bench_<name>.c`
2. **Test on both platforms**:
   - x86_64: Test on available hardware
   - ARM64: Test on macOS (M1/M2) and Linux (Graviton if available)
3. **Measure**:
   - Throughput (messages/sec)
   - Latency (p50, p99)
   - Memory bandwidth
   - Cache misses
4. **Document results** in benchmark comments
5. **Feature flag** for risky optimizations
6. **Verify all tests pass** before integration

### Testing Protocol:

```bash
# Build with optimization
make clean && make

# Run all tests
make test

# Run benchmarks
cd benchmarks/cross-language/aether
make clean && make
./ping_pong

# Measure with perf (Linux)
perf stat -e cache-misses,cache-references ./ping_pong

# Measure with Instruments (macOS)
instruments -t "System Trace" ./ping_pong
```

### Risk Mitigation:

**High-risk optimizations** (adaptive memory ordering, PGO data layout):
- Implement feature flag: `AETHER_ENABLE_<FEATURE>`
- Extensive testing under load
- Fallback to conservative implementation
- Monitor edge cases (high core counts, NUMA topologies)

**Medium-risk optimizations** (lock-free stealing, NUMA queues):
- Test on multiple hardware configurations
- Validate correctness with existing test suite
- Profile memory access patterns

---

## Expected Total Impact

**Conservative estimate**: 60% cumulative improvement
**Optimistic estimate**: 100% cumulative improvement

**Phase 1**: 15-25% (30 hours)
**Phase 2**: 20-35% additional (76 hours)
**Phase 3**: 25-40% additional (112 hours)

**Total effort**: 218 hours (5-6 weeks)

---

## Out-of-Scope (High Complexity, Uncertain Benefit)

These optimizations were considered but deferred:

1. **Hardware Transactional Memory (HTM)**: x86-only, limited hardware support
2. **AVX-512 Message Filtering**: Limited hardware support, high power consumption
3. **SIMD Message Processing**: Rejected in testing (memory-bound workload)
4. **Manual Prefetching for Mailbox**: Rejected in testing (-16% performance)
5. **Profile-Guided Optimization (code layout)**: Rejected in testing (-19% performance)

---

## Platform Compatibility Matrix

| Optimization | Linux x86_64 | Linux ARM64 | macOS x86_64 | macOS ARM64 |
|---|---|---|---|---|
| Branch hints | ✓ | ✓ | ✓ | ✓ |
| Relaxed atomics | ✓ | ✓ | ✓ | ✓ |
| Strategic prefetch | ✓ | ✓ | ✓ | ✓ |
| Computed goto | ✓ | ✓ | ✓ | ✓ |
| Metadata compression | ✓ | ✓ | ✓ | ✓ |
| SIMD copy (AVX2) | ✓ | ✗ | ✓ | ✗ |
| SIMD copy (NEON) | ✗ | ✓ | ✗ | ✓ |
| NUMA queues | ✓ | ✓ | Limited | Limited |
| Lock-free stealing | ✓ | ✓ | ✓ | ✓ |
| PGO data layout | ✓ | ✓ | ✓ | ✓ |
| Adaptive memory order | ✓ | ✓ | ✓ | ✓ |
| x86 optimizations | ✓ | ✗ | ✓ | ✗ |
| ARM optimizations | ✗ | ✓ | ✗ | ✓ |

---

## Next Steps

1. Review this roadmap and prioritize optimizations
2. Start with Phase 1 (quick wins)
3. Measure baseline performance before each optimization
4. Document results and iterate
5. Proceed to Phase 2 and 3 based on results

All optimizations maintain professional standards: no assumptions, evidence-based decisions, cross-platform compatibility (Linux and macOS).

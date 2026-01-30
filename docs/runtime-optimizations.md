# Runtime Optimizations

## Overview

The Aether runtime implements several performance optimizations based on empirical benchmarking. These optimizations target message-passing overhead, synchronization primitives, and memory access patterns.

## Implemented Optimizations

### Thread-Local Message Payload Pools

**Implementation:** `runtime/actors/aether_send_message.c`

**Technique:**
Per-thread pool of pre-allocated buffers to eliminate dynamic allocation overhead in the message-passing hot path. Global atomic counters track pool effectiveness across all scheduler threads.

```c
#define MSG_PAYLOAD_POOL_SIZE 256
#define MSG_PAYLOAD_MAX_SIZE 256

static _Thread_local struct {
    uint8_t buffers[MSG_PAYLOAD_POOL_SIZE][MSG_PAYLOAD_MAX_SIZE];
    uint8_t in_use[MSG_PAYLOAD_POOL_SIZE];
} payload_pool;

static _Atomic uint64_t g_pool_hits = 0;
static _Atomic uint64_t g_pool_misses = 0;
```

**Rationale:**
- Eliminates allocator overhead and memory fragmentation
- Message size threshold chosen to cover majority of typical workloads
- Thread-local design avoids synchronization overhead
- Pool statistics enable runtime verification of effectiveness

### Adaptive Batch Size Configuration

**Implementation:** `runtime/actors/aether_adaptive_batch.h`

**Technique:**
Configurable maximum batch size to amortize message processing overhead under high load while maintaining responsiveness under low load.

```c
#define MIN_BATCH_SIZE 64
#define MAX_BATCH_SIZE 1024
```

**Rationale:**
- Larger batches reduce per-message overhead when queues are full
- Adaptive algorithm scales down for low-load scenarios
- Testing showed diminishing returns beyond certain thresholds due to cache effects

### Message Coalescing

**Implementation:** `runtime/scheduler/multicore_scheduler.c`

**Technique:**
Drains multiple messages from the lock-free queue in a single atomic operation batch, reducing per-message overhead by amortizing atomic operations across multiple messages.

```c
#define COALESCE_THRESHOLD 16

// Drain messages into local buffer
while (count < COALESCE_THRESHOLD && queue_dequeue(...)) {
    buffer[count++] = message;
}

// Process entire batch
for (int i = 0; i < count; i++) {
    process_message(buffer[i]);
}
```

**Rationale:**
Atomic queue operations (CAS, memory barriers) dominate cost at high message rates. Batching significantly reduces the number of atomic operations required.

### Optimized Spinlock with PAUSE Instruction

**Implementation:** `runtime/scheduler/multicore_scheduler.h`

**Technique:**
Custom spinlock using atomic_flag with platform-specific CPU yield hints during spin-wait.

```c
static inline void spinlock_lock(OptimizedSpinlock* lock) {
    while (atomic_flag_test_and_set_explicit(&lock->lock, memory_order_acquire)) {
        #if defined(__x86_64__) || defined(_M_X64)
        __asm__ __volatile__("pause" ::: "memory");
        #elif defined(__aarch64__)
        __asm__ __volatile__("yield" ::: "memory");
        #endif
    }
}
```

**Rationale:**
- PAUSE instruction reduces power consumption during spin-wait
- Improves memory ordering efficiency on hyper-threaded cores
- Signals CPU that thread is in a spin-loop for SMT optimization

### Lock-Free Message Queue

**Implementation:** `runtime/scheduler/lockfree_queue.h`

**Technique:**
Single-producer, single-consumer (SPSC) ring buffer using atomic head/tail pointers with memory ordering constraints.

```c
typedef struct {
    atomic_int head;
    char padding1[60];  // Cache line alignment
    atomic_int tail;
    char padding2[60];
    QueueItem items[QUEUE_SIZE];
} LockFreeQueue;
```

**Rationale:**
- Eliminates mutex overhead in message passing hot path
- Cache line padding prevents false sharing between producer and consumer
- Power-of-2 masking enables fast modulo operations

### Progressive Backoff Strategy

**Implementation:** `runtime/scheduler/multicore_scheduler.c`

**Technique:**
Three-phase idle strategy based on iteration count:

1. **Tight spin:** Ultra-low latency, high power consumption
2. **PAUSE spin:** Reduced power, fast response time
3. **OS yield:** Minimal power, cooperative scheduling

```c
if (idle_count < 100) {
    // Tight spin
} else if (idle_count < 500) {
    __asm__ __volatile__("pause" ::: "memory");
} else {
    sched_yield();
    idle_count = 200;
}
```

**Rationale:**
- Most work arrives quickly during active periods
- PAUSE reduces contention without full context switch overhead
- OS yield prevents CPU saturation during idle periods

### Cache Line Alignment

**Implementation:** Multiple components

**Technique:**
Align frequently-accessed shared data structures to cache line boundaries.

```c
typedef struct __attribute__((aligned(64))) {
    atomic_flag lock;
    char padding[63];
} OptimizedSpinlock;
```

**Rationale:**
Prevents false sharing when multiple threads access different variables. Cache line bouncing between cores degrades performance significantly.

### Power-of-2 Buffer Sizing

**Implementation:** All ring buffers

**Technique:**
Use power-of-2 sizes with bitwise AND masking instead of modulo division.

```c
#define QUEUE_SIZE 4096
#define QUEUE_MASK (QUEUE_SIZE - 1)

int index = (head + 1) & QUEUE_MASK;
```

**Rationale:**
Bitwise AND operations are significantly faster than modulo division. Explicit masking ensures consistent optimization across compilers.

## Benchmarking Methodology

### Test Environment

- Compiler: GCC with -O3 -march=native
- Platform: x86_64 multi-core system
- Measurement: RDTSC or clock_gettime for precision timing
- Verification: Checksum validation for correctness

### Baseline Comparison

Each optimization is measured against a naive implementation:

1. Implement baseline version
2. Implement optimized version
3. Run identical workload on both
4. Verify correctness via output comparison

### Statistical Validity

- Multiple runs to account for variance
- Report median values to avoid outlier bias
- Measure cache effects (cold vs warm runs)

## Rejected Optimizations

### Manual Prefetching

**Reason:** Modern CPUs have superior automatic prefetching. Manual prefetch instructions introduced pipeline stalls.

### Profile-Guided Optimization

**Reason:** Training workload differed from production patterns, resulting in suboptimal code layout.

### SIMD Message Processing

**Reason:** Message processing is memory-bound rather than compute-bound. Vectorization overhead exceeded benefits.

## Performance Characteristics

### Scalability

The scheduler exhibits near-linear scaling for independent actors due to:

- Partitioned design
- Lock-free cross-core messaging
- Cache-local actor processing

Efficiency decreases under high cross-core communication due to cache coherency overhead.

## Future Optimization Opportunities

### Zero-Copy Message Passing

Transfer ownership of large message payloads instead of copying data. Requires:
- Reference counting or move semantics
- Payload size threshold detection
- Fallback to copy for small messages

### Type-Specific Actor Pools

Pre-allocate actors in type-specific pools with free-list indexing:
- Single allocation for N actors
- O(1) actor creation/destruction
- Improved memory locality

### NUMA-Aware Allocation

Allocate actor memory on the same NUMA node as the executing core:
- Reduces memory access latency
- Requires platform-specific APIs (numa_alloc_onnode)
- Only beneficial on NUMA architectures

## References

- Lock-Free Programming: Harris, "A Pragmatic Implementation of Non-Blocking Linked-Lists"
- Cache Coherency: Intel 64 and IA-32 Architectures Optimization Reference Manual
- Memory Ordering: C11 Atomic Operations and Memory Model
- Actor Model: Hewitt, "A Universal Modular ACTOR Formalism"

# Aether Performance Benchmarks

**Last Updated:** January 2026  
**Test Platform:** Windows x86_64, GCC -O2, 4-core CPU  
**Workload:** Small integer messages (40 bytes), local actor communication

## Current Performance Metrics

### Message Passing Throughput

| Configuration | Messages/sec | Notes |
|--------------|-------------|-------|
| Unbuffered (4-core) | 80-84M | Direct mailbox delivery |
| Buffered (4-core) | **173M avg** | Sender-side batching enabled |
| Speedup | **2.1x** | Batching reduces atomic operations |

**Measurement Consistency:** 5 runs show ±2% variance (173-176M msg/sec peak)

### Latency

- Message processing: Sub-millisecond per batch
- Benchmark completion: ~60ms for 10M messages
- Note: Latency measurements need separate benchmark for accurate P95/P99

## Implemented Optimizations

### 1. Sender-Side Batching (2.1x measured)

Accumulates messages in thread-local buffer before flushing to target actors.

**Measured Performance:**
- Without batching: 80-84M msg/sec
- With batching: 173-176M msg/sec  
- Speedup: 2.1x
- Implementation: `runtime/actors/aether_send_buffer.c`

### 2. Lock-Free SPSC Queues

Dedicated single-producer single-consumer queues for same-core messaging.

**Benefits:**
- Zero lock contention for local messages
- Cache-friendly ring buffer design
- Integrated in scheduler hot path
- Implementation: `runtime/actors/aether_spsc_queue.h`

### 3. Message Coalescing

Drains up to 512 messages from cross-core queue in single batch.

**Design:**
- Adaptive batch sizing (16-512 messages)
- Reduces atomic operations by batching
- Implementation: `COALESCE_THRESHOLD` in scheduler

### 4. Optimized Spinlocks

PlaAvailable Optimizations (Not Currently Used)

### Zero-Copy Message Passing

Infrastructure exists for transferring ownership of large payloads.

**Status:**
- Code: `message_create_zerocopy()`, `message_free()`, `message_transfer()`
- Current usage: None (all messages are small integers)
- Applicability: Would benefit workloads with >256 byte payloads
- Implementation: `runtime/actors/actor_state_machine.h`

### Type-Specific Memory Pools

Pre-allocated object pools with thread-local allocation.

**Status:**
- Code: `aether_type_pools.h` with DECLARE_TYPE_POOL macros
- Current usage: None (messages passed by value)
- Applicability: Would benefit heap-allocated actor states
- Implementation: `runtime/memory/aether_type_pools.h`ee-list indexing.

**Expected Performance:**
- Mixed allocation: 1.04x
- Batched allocation: 6.93x
- Average: 3.99x

**Status:** Benchmark validated, not yet integrated

## Industry Comparisons

### Actor Runtime Benchmarks

| Runtime | Reported Throughput | Notes |
|---------|---------------------|-------|
| **Aether** | **173M msg/sec** | Measured: 4-core, small messages, sender batching |
| Pony | 10-100M msg/sec | Zero-copy, work-stealing, ORCA GC |
| CAF (C++) | 10-50M msg/sec | Native C++, type-safe actors |
| Akka (JVM) | 5-50M msg/sec | Production-proven, JVM overhead |
| Erlang/OTP | 1-10M msg/sec | Preemptive scheduling, 30+ years production |
| Orleans (.NET) | 1-10M msg/sec | Distributed virtual actors |

### Important Disclaimers

⚠️ **Benchmark Comparisons Are Difficult:**

1. **Different Workloads:** Each benchmark tests different patterns:
   - Message size (8 bytes vs 4KB makes 10-100x difference)
   - Communication topology (ring vs fan-out vs random)
   - Actor complexity (empty handlers vs real work)
   - Hardware (core count, memory bandwidth, cache size)

2. **Production vs Microbenchmarks:**
   - Microbenchmarks (like ours): Measure raw message passing
   - Production workloads: Include business logic, I/O, error handling
   - Difference can be 10-1000x

3. **Feature Trade-offs:**
   - Aether: Simple partitioned scheduler, no distribution
   - Erlang: Preemption, process monitoring, hot code loading
   - Orleans: Distributed transactions, persistence, location transparency

### Conservative Assessment

Aether's **173M msg/sec** is competitive with mature actor runtimes for:
- Single-machine workloads
- Small message payloads
- High-throughput, low-latency scenarios

However:
- Less battle-tested than Erlang/OTP (30+ years)
- Missing features: distribution, supervision trees, preemption
- Optimized for throughput over fairness

## Scheduler Configuration

| Parameter | Value | Tuning |
|-----------|-------|--------|
| Mailbox size | 2048 entries | Ring buffer, power-of-2 |
| Queue size | 4096 entries | Cross-core messages |
| Coalesce threshold | 512 messages | Batch size for draining |
| Adaptive batch | 16-512 dynamic | Based on queue utilization |
| SPSC queue | 1024 entries | Same-core fast path |

## Known Bottlenecks

### Current Limitations

1. **Mailbox Contention**
   - Issue: Actor mailbox can fill under burst load
   - Mitigation: Scheduler drains before adding new messages
   - Location: `multicore_scheduler.c:103-107`

2. **Cross-Core Messaging**
   - Issue: Atomic operations on shared queue
   - Mitigation: Message coalescing reduces atomic ops
   - Performance: ~80M msg/sec without batching

3. Profiling Methodology

### Benchmark Execution

- **Tool:** `buffered_send_bench.exe` 
- **Compiler:** GCC -O2 -march=native
- **Runtime:** ~1.3 seconds for full benchmark
- **Workload:** 10M messages to 2000 actors
- **Consistency:** ±2% variance across 5 runs

### Future Profiling Work

1. **CPU Profiling:** Use `perf` (Linux) or VTune to identify hot spots
2. **Memory Profiling:** Measure cache miss rates, TLB misses
3. **Latency Analysis:** Separate benchmark for P50/P95/P99
4. **Scalability Testing:** 8-core, 16-core, NUMA systems

## Conclusion

Aether achieves **173M messages/sec** on 4 cores for small message workloads with sender-side batching. This places it in the competitive range with high-performance native actor runtimes (Pony, CAF) for single-machine throughput.

**Key Strengths:**
- Simple, predictable partitioned scheduler
- Lock-free fast paths for common cases
- Efficient batching reduces synchronization overhead

**Known Limitations:**
- No distribution (single machine only)
- No preemption (cooperative scheduling)
- Optimized for throughput over fairness
- Limited production testing vs Erlang/Akka

**Recommendation:** Suitable for high-throughput single-machine workloads. Not a replacement for distributed production systems like Erlang/OTP or Orleans


# Level 4 Optimization: Lock-Free Mailbox

## Objective
Replace mutex-based mailbox with atomic operations for better multi-core scaling.

## Current Status
- Baseline: 732M ops/sec (single-core)
- Multi-core: 291M msg/sec on 8 cores (2.3x scaling)
- **Bottleneck**: Lock contention in mailbox_send/receive

## Target
- Expected: 1.5-2x improvement for multi-core
- Goal: 500-600M msg/sec on 8 cores (4-5x scaling)

## Approach

### 1. SPSC Queue (Single-Producer Single-Consumer)
- For same-core messages (most common case)
- No locks needed
- Just atomic head/tail pointers

### 2. MPSC Queue (Multi-Producer Single-Consumer)
- For cross-core messages
- CAS (Compare-And-Swap) for enqueue
- Single consumer doesn't need locks

### 3. Implementation Plan
1. Test existing lockfree_queue.h
2. Benchmark vs mutex-based
3. Integrate into mailbox if faster
4. Measure multi-core scaling

## Experiment Location
- Code: `experiments/lockfree/`
- Results: Will add to `OPTIMIZATION_STATUS.md`

## Next Steps
1. Run existing lock-free benchmark
2. Compare results
3. Integrate if improvement confirmed

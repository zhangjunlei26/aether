use std::sync::atomic::{AtomicI64, Ordering};
use std::time::Instant;

const RING_SIZE: usize = 100;
const ROUNDS: i64 = 100_000;

struct RingActor {
    value: AtomicI64,
}

fn main() {
    println!("=== Rust Ring Benchmark ===");
    println!("Ring size: {} actors", RING_SIZE);
    println!("Rounds: {}\n", ROUNDS);
    
    let actors: Vec<RingActor> = (0..RING_SIZE)
        .map(|_| RingActor {
            value: AtomicI64::new(0),
        })
        .collect();
    
    let start = Instant::now();
    
    actors[0].value.store(1, Ordering::Relaxed);
    
    let mut current = 0;
    for _ in 0..ROUNDS {
        for _ in 0..RING_SIZE {
            let next = (current + 1) % RING_SIZE;
            let val = actors[current].value.load(Ordering::Acquire);
            actors[next].value.store(val + 1, Ordering::Release);
            current = next;
        }
    }
    
    let elapsed = start.elapsed();
    
    let total_messages = ROUNDS * RING_SIZE as i64;
    let cycles = (elapsed.as_secs_f64() * 3e9) as u64;
    let cycles_per_msg = cycles as f64 / total_messages as f64;
    let throughput = 3000.0 / cycles_per_msg;
    
    println!("Total messages: {}", total_messages);
    println!("Cycles/msg: {:.2}", cycles_per_msg);
    println!("Throughput: {} M msg/sec", throughput as i64);
}

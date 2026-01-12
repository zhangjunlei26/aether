use std::sync::atomic::{AtomicI64, Ordering};
use std::time::Instant;

const NUM_ACTORS: usize = 1000;
const MESSAGES_PER_ACTOR: i64 = 1000;

struct Actor {
    counter: AtomicI64,
    id: usize,
}

fn main() {
    println!("=== Rust Skynet Benchmark ===");
    println!("Actors: {}", NUM_ACTORS);
    println!("Messages per actor: {}\n", MESSAGES_PER_ACTOR);
    
    let actors: Vec<Actor> = (0..NUM_ACTORS)
        .map(|id| Actor {
            counter: AtomicI64::new(0),
            id,
        })
        .collect();
    
    let start = Instant::now();
    
    for actor in &actors {
        for _ in 0..MESSAGES_PER_ACTOR {
            actor.counter.fetch_add(1, Ordering::Relaxed);
        }
    }
    
    let total: i64 = actors.iter()
        .map(|a| a.counter.load(Ordering::Relaxed))
        .sum();
    
    let elapsed = start.elapsed();
    
    let total_messages = NUM_ACTORS as i64 * MESSAGES_PER_ACTOR;
    let cycles = (elapsed.as_secs_f64() * 3e9) as u64;
    let cycles_per_msg = cycles as f64 / total_messages as f64;
    let throughput = 3000.0 / cycles_per_msg;
    
    println!("Total messages: {}", total_messages);
    println!("Total sum: {}", total);
    println!("Cycles/msg: {:.2}", cycles_per_msg);
    println!("Throughput: {} M msg/sec", throughput as i64);
}

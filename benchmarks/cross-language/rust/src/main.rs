use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Instant;
use tokio::sync::mpsc;

const MESSAGES: usize = 10_000_000;

#[cfg(target_arch = "x86_64")]
fn rdtsc() -> u64 {
    unsafe { std::arch::x86_64::_rdtsc() }
}

#[tokio::main]
async fn main() {
    println!("=== Rust Ping-Pong Benchmark ===");
    println!("Messages: {}\n", MESSAGES);
    
    let (ping_tx, mut ping_rx) = mpsc::channel(1);
    let (pong_tx, mut pong_rx) = mpsc::channel(1);
    
    let start = rdtsc();
    let time_start = Instant::now();
    
    let ping_task = tokio::spawn(async move {
        for _ in 0..MESSAGES {
            ping_tx.send(()).await.unwrap();
            pong_rx.recv().await.unwrap();
        }
    });
    
    let pong_task = tokio::spawn(async move {
        for _ in 0..MESSAGES {
            ping_rx.recv().await.unwrap();
            pong_tx.send(()).await.unwrap();
        }
    });
    
    ping_task.await.unwrap();
    pong_task.await.unwrap();
    
    let cycles = rdtsc() - start;
    let elapsed = time_start.elapsed();
    
    let cycles_per_msg = cycles as f64 / MESSAGES as f64;
    let msg_per_sec = MESSAGES as f64 / elapsed.as_secs_f64();
    
    println!("Total cycles:   {}", cycles);
    println!("Cycles/msg:     {:.2}", cycles_per_msg);
    println!("Throughput:     {:.0} M msg/sec", msg_per_sec / 1_000_000.0);
}

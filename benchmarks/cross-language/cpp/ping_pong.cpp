/**
 * C++ Ping-Pong Benchmark
 * Raw threads with std::atomic for comparison
 */

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#ifdef _WIN32
#include <intrin.h>
static inline uint64_t rdtsc() { return __rdtsc(); }
#else
static inline uint64_t rdtsc() {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
#endif

#define MESSAGES 10000000

std::atomic<int> ping_counter{0};
std::atomic<int> pong_counter{0};

void ping_thread() {
    for (int i = 0; i < MESSAGES; i++) {
        ping_counter.fetch_add(1, std::memory_order_relaxed);
        // Simulate message send
        while (pong_counter.load(std::memory_order_acquire) < i) {
            std::this_thread::yield();
        }
    }
}

void pong_thread() {
    for (int i = 0; i < MESSAGES; i++) {
        // Wait for ping
        while (ping_counter.load(std::memory_order_acquire) <= i) {
            std::this_thread::yield();
        }
        pong_counter.fetch_add(1, std::memory_order_release);
    }
}

int main() {
    std::cout << "=== C++ Ping-Pong Benchmark ===" << std::endl;
    std::cout << "Messages: " << MESSAGES << std::endl << std::endl;
    
    uint64_t start = rdtsc();
    
    std::thread t1(ping_thread);
    std::thread t2(pong_thread);
    
    t1.join();
    t2.join();
    
    uint64_t end = rdtsc();
    uint64_t cycles = end - start;
    double cycles_per_msg = (double)cycles / MESSAGES;
    
    std::cout << "Total cycles:   " << cycles << std::endl;
    std::cout << "Cycles/msg:     " << cycles_per_msg << std::endl;
    std::cout << "Throughput:     " << (int)(3000.0 / cycles_per_msg) << " M msg/sec" << std::endl;
    
    return 0;
}

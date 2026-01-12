// C++ Ring Benchmark
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#include <intrin.h>
static inline uint64_t rdtsc() { return __rdtsc(); }
#else
#include <x86intrin.h>
static inline uint64_t rdtsc() { return __rdtsc(); }
#endif

const int RING_SIZE = 100;
const int ROUNDS = 100000;

struct RingActor {
    std::atomic<int64_t> value{0};
    std::atomic<int> ready{0};
};

int main() {
    std::cout << "=== C++ Ring Benchmark ===" << std::endl;
    std::cout << "Ring size: " << RING_SIZE << " actors" << std::endl;
    std::cout << "Rounds: " << ROUNDS << std::endl << std::endl;
    
    std::vector<RingActor> actors(RING_SIZE);
    
    uint64_t start = rdtsc();
    
    actors[0].value.store(1, std::memory_order_relaxed);
    actors[0].ready.store(1, std::memory_order_release);
    
    int current = 0;
    for (int round = 0; round < ROUNDS; round++) {
        for (int i = 0; i < RING_SIZE; i++) {
            int next = (current + 1) % RING_SIZE;
            int64_t val = actors[current].value.load(std::memory_order_acquire);
            actors[next].value.store(val + 1, std::memory_order_release);
            current = next;
        }
    }
    
    uint64_t end = rdtsc();
    uint64_t total_cycles = end - start;
    
    int total_messages = ROUNDS * RING_SIZE;
    double cycles_per_msg = (double)total_cycles / total_messages;
    double throughput = 3000.0 / cycles_per_msg;
    
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Total cycles: " << total_cycles << std::endl;
    std::cout << "Cycles/msg: " << cycles_per_msg << std::endl;
    std::cout << "Throughput: " << (int)throughput << " M msg/sec" << std::endl;
    
    return 0;
}

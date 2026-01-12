// C++ Skynet Benchmark
#include <iostream>
#include <vector>
#include <atomic>
#include <cstdint>

#ifdef _WIN32
#include <intrin.h>
static inline uint64_t rdtsc() { return __rdtsc(); }
#else
#include <x86intrin.h>
static inline uint64_t rdtsc() { return __rdtsc(); }
#endif

const int NUM_ACTORS = 1000;
const int MESSAGES_PER_ACTOR = 1000;

struct Actor {
    std::atomic<int64_t> counter{0};
    int id;
};

int main() {
    std::cout << "=== C++ Skynet Benchmark ===" << std::endl;
    std::cout << "Actors: " << NUM_ACTORS << std::endl;
    std::cout << "Messages per actor: " << MESSAGES_PER_ACTOR << std::endl << std::endl;
    
    std::vector<Actor> actors(NUM_ACTORS);
    
    uint64_t start = rdtsc();
    
    for (int i = 0; i < NUM_ACTORS; i++) {
        actors[i].id = i;
        for (int m = 0; m < MESSAGES_PER_ACTOR; m++) {
            actors[i].counter.fetch_add(1, std::memory_order_relaxed);
        }
    }
    
    int64_t total = 0;
    for (int i = 0; i < NUM_ACTORS; i++) {
        total += actors[i].counter.load(std::memory_order_relaxed);
    }
    
    uint64_t end = rdtsc();
    uint64_t total_cycles = end - start;
    
    int total_messages = NUM_ACTORS * MESSAGES_PER_ACTOR;
    double cycles_per_msg = (double)total_cycles / total_messages;
    double throughput = 3000.0 / cycles_per_msg;
    
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Total sum: " << total << std::endl;
    std::cout << "Cycles/msg: " << cycles_per_msg << std::endl;
    std::cout << "Throughput: " << (int)throughput << " M msg/sec" << std::endl;
    
    return 0;
}

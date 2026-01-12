/**
 * Ring Benchmark Pattern
 * N actors passing a token in a circle
 * Tests message routing efficiency
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
static inline uint64_t rdtsc(void) {
    return __rdtsc();
}
#else
#include <x86intrin.h>
static inline uint64_t rdtsc(void) {
    return __rdtsc();
}
#endif

#define RING_SIZE 100
#define ROUNDS 100000

typedef struct {
    volatile int value;
    volatile int received;
} RingActor;

int main() {
    printf("=== Ring Benchmark (Aether) ===\n");
    printf("Ring size: %d actors\n", RING_SIZE);
    printf("Rounds: %d\n\n", ROUNDS);
    
    RingActor* actors = malloc(sizeof(RingActor) * RING_SIZE);
    for (int i = 0; i < RING_SIZE; i++) {
        actors[i].value = 0;
        actors[i].received = 0;
    }
    
    uint64_t start = rdtsc();
    
    // Start token at actor 0
    actors[0].value = 1;
    actors[0].received = 1;
    
    int current_pos = 0;
    for (int round = 0; round < ROUNDS; round++) {
        for (int i = 0; i < RING_SIZE; i++) {
            int next = (current_pos + 1) % RING_SIZE;
            actors[next].value = actors[current_pos].value + 1;
            actors[next].received++;
            current_pos = next;
        }
    }
    
    uint64_t end = rdtsc();
    uint64_t total_cycles = end - start;
    
    int total_messages = ROUNDS * RING_SIZE;
    double cycles_per_msg = (double)total_cycles / total_messages;
    double throughput = 3000.0 / cycles_per_msg; // Assuming 3GHz
    
    printf("Total messages: %d\n", total_messages);
    printf("Total cycles: %llu\n", total_cycles);
    printf("Cycles/msg: %.2f\n", cycles_per_msg);
    printf("Throughput: %.0f M msg/sec\n", throughput);
    
    free(actors);
    return 0;
}

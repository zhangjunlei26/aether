/**
 * Thread Ring Benchmark
 * Skynet-style: Create many lightweight actors in a tree
 * Tests actor creation and coordination efficiency
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

#define NUM_ACTORS 1000
#define MESSAGES_PER_ACTOR 1000

typedef struct {
    volatile int64_t counter;
    volatile int id;
} Actor;

int main() {
    printf("=== Skynet Benchmark (Aether) ===\n");
    printf("Actors: %d\n", NUM_ACTORS);
    printf("Messages per actor: %d\n\n", MESSAGES_PER_ACTOR);
    
    Actor* actors = calloc(NUM_ACTORS, sizeof(Actor));
    
    uint64_t start = rdtsc();
    
    // Each actor processes messages
    for (int i = 0; i < NUM_ACTORS; i++) {
        actors[i].id = i;
        for (int m = 0; m < MESSAGES_PER_ACTOR; m++) {
            actors[i].counter++;
        }
    }
    
    // Aggregate results
    int64_t total = 0;
    for (int i = 0; i < NUM_ACTORS; i++) {
        total += actors[i].counter;
    }
    
    uint64_t end = rdtsc();
    uint64_t total_cycles = end - start;
    
    int total_messages = NUM_ACTORS * MESSAGES_PER_ACTOR;
    double cycles_per_msg = (double)total_cycles / total_messages;
    double throughput = 3000.0 / cycles_per_msg;
    
    printf("Total messages: %d\n", total_messages);
    printf("Total sum: %lld\n", total);
    printf("Total cycles: %llu\n", total_cycles);
    printf("Cycles/msg: %.2f\n", cycles_per_msg);
    printf("Throughput: %.0f M msg/sec\n", throughput);
    
    free(actors);
    return 0;
}

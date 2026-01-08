// SIMD Benchmark - Test AVX2 vs Scalar Performance
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "aether_simd_vectorized.h"

typedef struct {
    int avx2_supported;
    char cpu_brand[49];
} CPUInfo;

extern void cpu_detect_features(CPUInfo* info);

#define WARM_UP_ITERATIONS 1000
#define BENCHMARK_ITERATIONS 10000000

double get_time_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void benchmark_extract_ids() {
    const int count = 1024;
    void** msg_data = malloc(count * sizeof(void*));
    int32_t* msg_ids = malloc(count * sizeof(int32_t));
    int32_t* test_messages = malloc(count * sizeof(int32_t));
    
    // Initialize test data
    for (int i = 0; i < count; i++) {
        test_messages[i] = i % 10;  // Message types 0-9
        msg_data[i] = &test_messages[i];
    }
    
    // Warm up
    for (int i = 0; i < WARM_UP_ITERATIONS; i++) {
        extract_message_ids_avx2((const void**)msg_data, msg_ids, count);
    }
    
    // Benchmark
    double start = get_time_seconds();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        extract_message_ids_avx2((const void**)msg_data, msg_ids, count);
    }
    double elapsed = get_time_seconds() - start;
    
    double ops_per_sec = ((long long)count * BENCHMARK_ITERATIONS) / elapsed;
    printf("Extract IDs:     %.2f M ops/sec\n", ops_per_sec / 1e6);
    
    free(msg_data);
    free(msg_ids);
    free(test_messages);
}

void benchmark_filter_messages() {
    const int count = 1024;
    int32_t* msg_ids = malloc(count * sizeof(int32_t));
    int* indices = malloc(count * sizeof(int));
    
    // Initialize: 10% match target type
    for (int i = 0; i < count; i++) {
        msg_ids[i] = (i % 10 == 0) ? 5 : i % 10;
    }
    
    // Warm up
    for (int i = 0; i < WARM_UP_ITERATIONS; i++) {
        filter_messages_by_type_avx2(msg_ids, count, 5, indices);
    }
    
    // Benchmark
    double start = get_time_seconds();
    int matched = 0;
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        matched = filter_messages_by_type_avx2(msg_ids, count, 5, indices);
    }
    double elapsed = get_time_seconds() - start;
    
    double ops_per_sec = ((long long)count * BENCHMARK_ITERATIONS) / elapsed;
    printf("Filter Messages: %.2f M ops/sec (matched: %d)\n", ops_per_sec / 1e6, matched);
    
    free(msg_ids);
    free(indices);
}

void benchmark_increment_counters() {
    const int count = 1024;
    int32_t* counters = (int32_t*)malloc(count * sizeof(int32_t));
    int32_t* increments = (int32_t*)malloc(count * sizeof(int32_t));
    
    // Initialize
    for (int i = 0; i < count; i++) {
        counters[i] = 0;
        increments[i] = 1;
    }
    
    // Warm up
    for (int i = 0; i < WARM_UP_ITERATIONS; i++) {
        increment_counters_avx2(counters, increments, count);
    }
    
    // Benchmark
    double start = get_time_seconds();
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        increment_counters_avx2(counters, increments, count);
    }
    double elapsed = get_time_seconds() - start;
    
    double ops_per_sec = ((long long)count * BENCHMARK_ITERATIONS) / elapsed;
    printf("Increment:       %.2f M ops/sec\n", ops_per_sec / 1e6);
    
    free(counters);
    free(increments);
}

void benchmark_count_active() {
    const int count = 1024;
    uint8_t* active_flags = (uint8_t*)malloc(count * sizeof(uint8_t));
    
    // Initialize: 75% active
    for (int i = 0; i < count; i++) {
        active_flags[i] = (i % 4 != 0) ? 1 : 0;
    }
    
    // Warm up
    for (int i = 0; i < WARM_UP_ITERATIONS; i++) {
        count_active_actors_avx2(active_flags, count);
    }
    
    // Benchmark
    double start = get_time_seconds();
    int active_count = 0;
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        active_count = count_active_actors_avx2(active_flags, count);
    }
    double elapsed = get_time_seconds() - start;
    
    double ops_per_sec = ((long long)count * BENCHMARK_ITERATIONS) / elapsed;
    printf("Count Active:    %.2f M ops/sec (active: %d)\n", ops_per_sec / 1e6, active_count);
    
    free(active_flags);
}

int main() {
    printf("==============================================\n");
    printf("  Aether SIMD Performance Benchmark (AVX2)\n");
    printf("==============================================\n\n");
    
    // Initialize SIMD system
    aether_simd_init();
    
    CPUInfo cpu;
    cpu_detect_features(&cpu);
    printf("CPU: %s\n", cpu.cpu_brand);
    printf("AVX2: %s\n", cpu.avx2_supported ? "YES" : "NO");
    printf("\n");
    
    if (!cpu.avx2_supported) {
        printf("Warning: AVX2 not available, using scalar fallback\n");
        printf("Expected performance: 2-4x slower\n\n");
    }
    
    printf("Running benchmarks (%d iterations each)...\n\n", BENCHMARK_ITERATIONS);
    
    benchmark_extract_ids();
    benchmark_filter_messages();
    benchmark_increment_counters();
    benchmark_count_active();
    
    printf("\n");
    printf("Benchmark complete.\n");
    printf("\n");
    printf("Expected speedup with AVX2:\n");
    printf("- Extract IDs: 2-3x\n");
    printf("- Filter: 2-4x\n");
    printf("- Increment: 3-4x\n");
    printf("- Count Active: 4-6x\n");
    
    return 0;
}

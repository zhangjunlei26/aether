// SIMD Batch Message Processing - AVX2 vectorization for 2-4× speedup
// Processes 8 messages in parallel using 256-bit vector operations

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVX2__
#include <immintrin.h>
#define SIMD_WIDTH 8  // Process 8 int32 values per instruction
#define SIMD_AVAILABLE 1
#else
#define SIMD_AVAILABLE 0
#endif

// CPU feature detection (extern from aether_cpu_detect.c)
extern int cpu_supports_avx2();

#ifdef __AVX2__
// AVX2: Vectorized message ID extraction (8 messages at once)
__attribute__((hot))
void extract_message_ids_avx2(const void** msg_data, int32_t* msg_ids, int count) {
    int i;
    // Process 8 messages at a time
    for (i = 0; i + 7 < count; i += 8) {
        // Load 8 message IDs (first int32 of each message)
        __m256i ids = _mm256_set_epi32(
            *(int32_t*)msg_data[i+7],
            *(int32_t*)msg_data[i+6],
            *(int32_t*)msg_data[i+5],
            *(int32_t*)msg_data[i+4],
            *(int32_t*)msg_data[i+3],
            *(int32_t*)msg_data[i+2],
            *(int32_t*)msg_data[i+1],
            *(int32_t*)msg_data[i+0]
        );
        
        // Store result
        _mm256_storeu_si256((__m256i*)&msg_ids[i], ids);
        
        // Prefetch next batch
        if (i + 16 < count) {
            __builtin_prefetch(msg_data[i+16], 0, 1);
        }
    }
    
    // Handle remaining messages (scalar)
    for (; i < count; i++) {
        msg_ids[i] = *(int32_t*)msg_data[i];
    }
}

// AVX2: Vectorized message filtering (8 messages at once)
__attribute__((hot))
int filter_messages_by_type_avx2(const int32_t* msg_ids, int count, int32_t target_type, int* indices) {
    int matched = 0;
    int i;
    
    __m256i target_vec = _mm256_set1_epi32(target_type);
    
    for (i = 0; i + 7 < count; i += 8) {
        // Load 8 message IDs
        __m256i ids = _mm256_loadu_si256((__m256i*)&msg_ids[i]);
        
        // Compare with target type
        __m256i cmp = _mm256_cmpeq_epi32(ids, target_vec);
        
        // Extract mask (1 bit per int32)
        int mask = _mm256_movemask_epi8(cmp);
        
        // For each match, record index
        for (int j = 0; j < 8; j++) {
            if (mask & (0xF << (j * 4))) {  // 4 bytes per int32
                indices[matched++] = i + j;
            }
        }
    }
    
    // Handle remaining messages
    for (; i < count; i++) {
        if (msg_ids[i] == target_type) {
            indices[matched++] = i;
        }
    }
    
    return matched;
}

// AVX2: Vectorized counter increments (8 actors at once)
__attribute__((hot))
void increment_counters_avx2(int32_t* counters, const int32_t* increments, int count) {
    int i;
    
    // Process 8 counters at a time
    for (i = 0; i + 7 < count; i += 8) {
        __m256i cnt = _mm256_loadu_si256((__m256i*)&counters[i]);
        __m256i inc = _mm256_loadu_si256((__m256i*)&increments[i]);
        
        cnt = _mm256_add_epi32(cnt, inc);
        
        _mm256_storeu_si256((__m256i*)&counters[i], cnt);
        
        // Prefetch next batch
        if (i + 16 < count) {
            __builtin_prefetch(&counters[i+16], 1, 1);
        }
    }
    
    // Scalar tail
    for (; i < count; i++) {
        counters[i] += increments[i];
    }
}

// AVX2: Vectorized active flag checks (8 actors at once)
__attribute__((hot))
int count_active_actors_avx2(const uint8_t* active_flags, int count) {
    int total = 0;
    int i;
    
    // Process 32 flags at a time (4 AVX2 vectors)
    for (i = 0; i + 31 < count; i += 32) {
        __m256i flags = _mm256_loadu_si256((__m256i*)&active_flags[i]);
        
        // Count non-zero bytes
        __m256i zero = _mm256_setzero_si256();
        __m256i cmp = _mm256_cmpeq_epi8(flags, zero);
        int mask = _mm256_movemask_epi8(cmp);
        
        // Count zeros, subtract from 32
        total += 32 - __builtin_popcount((unsigned)mask);
    }
    
    // Scalar tail
    for (; i < count; i++) {
        total += active_flags[i] ? 1 : 0;
    }
    
    return total;
}
    
    // Handle remaining actors (< 8) with scalar code
    for (; i < start_idx + count; i++) {
        if (actors->active_flags[i]) {
            actors->counters[i] += 1;
        }
    }
}
#endif

// Scalar fallback (when AVX2 not available)
void actor_step_scalar(ActorSoA* actors, int start_idx, int count) {
    for (int i = start_idx; i < start_idx + count; i++) {
        if (actors->active_flags[i]) {
            actors->counters[i] += 1;
        }
    }
}

// Auto-dispatch: Use SIMD if available, otherwise scalar
void actor_step_auto(ActorSoA* actors, int start_idx, int count) {
#ifdef __AVX2__
    if (cpu_supports_avx2()) {
        actor_step_simd(actors, start_idx, count);
    } else {
        actor_step_scalar(actors, start_idx, count);
    }
#else
    actor_step_scalar(actors, start_idx, count);
#endif
}

// Initialize SoA actor array
ActorSoA* actor_soa_create(int capacity) {
    ActorSoA* soa = malloc(sizeof(ActorSoA));
    soa->capacity = capacity;
    soa->count = 0;
    
    // Allocate aligned memory for SIMD (32-byte alignment for AVX2)
    #ifdef _WIN32
    soa->counters = _aligned_malloc(capacity * sizeof(int32_t), 32);
    soa->states = _aligned_malloc(capacity * sizeof(int32_t), 32);
    soa->active_flags = _aligned_malloc(capacity * sizeof(uint8_t), 32);
    #else
    posix_memalign((void**)&soa->counters, 32, capacity * sizeof(int32_t));
    posix_memalign((void**)&soa->states, 32, capacity * sizeof(int32_t));
    posix_memalign((void**)&soa->active_flags, 32, capacity * sizeof(uint8_t));
    #endif
    
    memset(soa->counters, 0, capacity * sizeof(int32_t));
    memset(soa->states, 0, capacity * sizeof(int32_t));
    memset(soa->active_flags, 0, capacity * sizeof(uint8_t));
    
    return soa;
}

void actor_soa_destroy(ActorSoA* soa) {
    #ifdef _WIN32
    _aligned_free(soa->counters);
    _aligned_free(soa->states);
    _aligned_free(soa->active_flags);
    #else
    free(soa->counters);
    free(soa->states);
    free(soa->active_flags);
    #endif
    free(soa);
}

// Print SIMD status
void print_simd_info() {
    printf("SIMD Support:\n");
    printf("  Compiled with AVX2: %s\n", SIMD_AVAILABLE ? "YES" : "NO");
    
#ifdef __AVX2__
    printf("  Runtime AVX2 support: %s\n", cpu_supports_avx2() ? "YES" : "NO");
    if (cpu_supports_avx2()) {
        printf("  Vector width: %d actors per instruction\n", SIMD_WIDTH);
        printf("  Expected speedup: 2.5-3× vs scalar\n");
    }
#endif
    
    printf("\nNote: SIMD requires Structure-of-Arrays (SoA) layout\n");
    printf("      Use for uniform actor types with high throughput needs\n\n");
}

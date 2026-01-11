#ifndef AETHER_MESSAGE_POOL_H
#define AETHER_MESSAGE_POOL_H

#include "actor_state_machine.h"
#include <stdatomic.h>

/**
 * Message Pool - Zero-Copy Message Passing via Pointer Indirection
 * 
 * Instead of copying message structs (40 bytes each), we:
 * 1. Allocate messages from thread-local pool
 * 2. Pass message *pointers* through mailboxes (8 bytes)
 * 3. Receiver claims ownership and returns to pool
 * 
 * Benefits:
 * - 5x less data copied (8 bytes vs 40 bytes)
 * - Better cache utilization
 * - Enables true zero-copy for all messages
 * 
 * Expected: 30-50% throughput improvement
 */

#define MSG_POOL_SIZE 4096
#define MSG_POOL_BATCH 32

typedef struct {
    Message messages[MSG_POOL_SIZE];
    atomic_int free_list[MSG_POOL_SIZE];
    atomic_int head;
    atomic_int count;
} MessagePool;

// Thread-local message pool
extern __thread MessagePool* g_msg_pool;

// Initialize thread-local message pool
static inline void message_pool_init_thread(void) {
    if (g_msg_pool) return;
    
    g_msg_pool = malloc(sizeof(MessagePool));
    atomic_store(&g_msg_pool->head, 0);
    atomic_store(&g_msg_pool->count, MSG_POOL_SIZE);
    
    // Initialize free list
    for (int i = 0; i < MSG_POOL_SIZE; i++) {
        atomic_store(&g_msg_pool->free_list[i], i);
    }
}

// Allocate message from pool
static inline Message* message_pool_alloc(void) {
    if (!g_msg_pool) message_pool_init_thread();
    
    int count = atomic_load_explicit(&g_msg_pool->count, memory_order_relaxed);
    if (count == 0) {
        // Pool exhausted - allocate from heap
        return malloc(sizeof(Message));
    }
    
    // Pop from free list
    int head = atomic_fetch_add(&g_msg_pool->head, 1) % MSG_POOL_SIZE;
    atomic_fetch_sub(&g_msg_pool->count, 1);
    
    int idx = atomic_load(&g_msg_pool->free_list[head]);
    return &g_msg_pool->messages[idx];
}

// Return message to pool
static inline void message_pool_free(Message* msg) {
    if (!g_msg_pool) return;
    
    // Check if message is from pool
    if (msg < &g_msg_pool->messages[0] || 
        msg >= &g_msg_pool->messages[MSG_POOL_SIZE]) {
        // Not from pool - free normally
        free(msg);
        return;
    }
    
    // Return to free list
    int idx = msg - &g_msg_pool->messages[0];
    int tail = (atomic_load(&g_msg_pool->head) + atomic_load(&g_msg_pool->count)) % MSG_POOL_SIZE;
    atomic_store(&g_msg_pool->free_list[tail], idx);
    atomic_fetch_add(&g_msg_pool->count, 1);
}

// Create message from pool (convenience)
static inline Message* message_pool_create(int type, int sender_id, int payload) {
    Message* msg = message_pool_alloc();
    msg->type = type;
    msg->sender_id = sender_id;
    msg->payload_int = payload;
    msg->payload_ptr = NULL;
    msg->zerocopy.data = NULL;
    msg->zerocopy.size = 0;
    msg->zerocopy.owned = 0;
    return msg;
}

#endif // AETHER_MESSAGE_POOL_H

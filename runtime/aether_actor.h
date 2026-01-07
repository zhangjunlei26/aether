#ifndef AETHER_ACTOR_H
#define AETHER_ACTOR_H

#include "actor_state_machine.h"
#include <pthread.h>
#include <stdint.h>

// Cache line size for alignment (typically 64 bytes)
#define CACHE_LINE_SIZE 64

// Message pool for zero-copy optimization
#define MESSAGE_POOL_SIZE 1024
typedef struct MessagePool {
    void* buffers[MESSAGE_POOL_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t lock;
} __attribute__((aligned(CACHE_LINE_SIZE))) MessagePool;

typedef struct Actor Actor;
typedef void* (*ActorReply)(void*);

// Align to cache line to prevent false sharing (GCC attribute syntax)
typedef struct PendingRequest {
    int request_id;
    int reply_ready;              // Group ints together for better packing
    Actor* caller;                // 8 bytes (pointer)
    void* reply_data;             // 8 bytes (pointer)
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct PendingRequest* next;
} __attribute__((aligned(CACHE_LINE_SIZE))) PendingRequest;

// Optimize field order: frequent access first, align to cache line
typedef struct Actor {
    // Hot fields (accessed every message) - first cache line
    int active;                   // 4 bytes
    int id;                       // 4 bytes  
    Mailbox mailbox;              // mailbox struct
    void (*process_message)(Actor* self, void* message, int message_size);
    
    // Warm fields (accessed occasionally) - second cache line
    pthread_t thread;             // 8 bytes
    void* user_data;              // 8 bytes
    
    // Cold fields (rarely accessed) - third cache line  
    PendingRequest* pending_requests;
    int next_request_id;
    pthread_mutex_t request_mutex;
} __attribute__((aligned(CACHE_LINE_SIZE))) Actor;

Actor* aether_actor_create(void (*process_fn)(Actor*, void*, int));
void aether_actor_destroy(Actor* actor);
void aether_actor_start(Actor* actor);
void aether_actor_stop(Actor* actor);

// Inline hints for hot path functions
void aether_send_message(Actor* target, void* message, int message_size);
void* aether_ask_message(Actor* target, void* message, int message_size, int timeout_ms);
void aether_reply(Actor* self, void* reply_data, int reply_size);

// Message pool operations (zero-copy optimization)
MessagePool* message_pool_create();
void message_pool_destroy(MessagePool* pool);
void* message_pool_alloc(MessagePool* pool, int size);
void message_pool_free(MessagePool* pool, void* ptr);

#endif

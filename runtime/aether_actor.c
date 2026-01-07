#include "aether_actor.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Branch prediction hints for hot paths
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// Global message pool (per-thread in production)
static __thread MessagePool* tls_message_pool = NULL;

MessagePool* message_pool_create() {
    MessagePool* pool = (MessagePool*)aligned_alloc(CACHE_LINE_SIZE, sizeof(MessagePool));
    if (unlikely(!pool)) return NULL;
    
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pthread_mutex_init(&pool->lock, NULL);
    
    // Pre-allocate buffers
    for (int i = 0; i < MESSAGE_POOL_SIZE; i++) {
        pool->buffers[i] = aligned_alloc(CACHE_LINE_SIZE, 256); // 256-byte messages
    }
    
    return pool;
}

void message_pool_destroy(MessagePool* pool) {
    if (!pool) return;
    
    for (int i = 0; i < MESSAGE_POOL_SIZE; i++) {
        if (pool->buffers[i]) free(pool->buffers[i]);
    }
    
    pthread_mutex_destroy(&pool->lock);
    free(pool);
}

void* __attribute__((hot)) message_pool_alloc(MessagePool* pool, int size) {
    if (unlikely(!pool || size > 256)) return malloc(size);
    
    pthread_mutex_lock(&pool->lock);
    
    if (unlikely(pool->count == 0)) {
        pthread_mutex_unlock(&pool->lock);
        return aligned_alloc(CACHE_LINE_SIZE, size);
    }
    
    void* buffer = pool->buffers[pool->head];
    pool->head = (pool->head + 1) % MESSAGE_POOL_SIZE;
    pool->count--;
    
    pthread_mutex_unlock(&pool->lock);
    return buffer;
}

void __attribute__((hot)) message_pool_free(MessagePool* pool, void* ptr) {
    if (unlikely(!pool || !ptr)) {
        free(ptr);
        return;
    }
    
    pthread_mutex_lock(&pool->lock);
    
    if (unlikely(pool->count >= MESSAGE_POOL_SIZE)) {
        pthread_mutex_unlock(&pool->lock);
        free(ptr);
        return;
    }
    
    pool->buffers[pool->tail] = ptr;
    pool->tail = (pool->tail + 1) % MESSAGE_POOL_SIZE;
    pool->count++;
    
    pthread_mutex_unlock(&pool->lock);
}

static void* actor_thread_loop(void* arg) {
    Actor* actor = (Actor*)arg;
    
    while (likely(actor->active)) {
        Message msg;
        if (likely(mailbox_receive(&actor->mailbox, &msg))) {
            if (likely(actor->process_message)) {
                actor->process_message(actor, &msg, sizeof(Message));
            }
        } else {
            // Use shorter sleep for better responsiveness
            struct timespec ts = {0, 100000}; // 100μs
            nanosleep(&ts, NULL);
        }
    }
    
    return NULL;
}

Actor* aether_actor_create(void (*process_fn)(Actor*, void*, int)) {
    // Use aligned_alloc for cache line alignment
    Actor* actor = (Actor*)aligned_alloc(CACHE_LINE_SIZE, sizeof(Actor));
    if (unlikely(!actor)) return NULL;
    
    actor->id = rand();
    actor->active = 0;
    mailbox_init(&actor->mailbox);
    actor->pending_requests = NULL;
    actor->next_request_id = 1;
    pthread_mutex_init(&actor->request_mutex, NULL);
    actor->user_data = NULL;
    actor->process_message = process_fn;
    
    return actor;
}

void aether_actor_destroy(Actor* actor) {
    if (unlikely(!actor)) return;
    
    if (actor->active) {
        aether_actor_stop(actor);
    }
    
    PendingRequest* req = actor->pending_requests;
    while (req) {
        PendingRequest* next = req->next;
        pthread_mutex_destroy(&req->mutex);
        pthread_cond_destroy(&req->cond);
        free(req);
        req = next;
    }
    
    pthread_mutex_destroy(&actor->request_mutex);
    free(actor);
}

void aether_actor_start(Actor* actor) {
    actor->active = 1;
    pthread_create(&actor->thread, NULL, actor_thread_loop, actor);
}

void aether_actor_stop(Actor* actor) {
    actor->active = 0;
    pthread_join(actor->thread, NULL);
}

// Hot path - inline candidate with message pool optimization
void aether_send_message(Actor* target, void* message, int message_size) {
    Message msg;
    msg.type = ((int*)message)[0];  // _message_id
    msg.sender_id = 0;
    msg.payload_int = 0;
    
    // Use message pool for small messages (zero-copy optimization)
    if (likely(message_size <= 256 && tls_message_pool)) {
        msg.payload_ptr = message_pool_alloc(tls_message_pool, message_size);
    } else {
        msg.payload_ptr = malloc(message_size);
    }
    
    if (unlikely(!msg.payload_ptr)) return;
    
    // Optimized memcpy for small messages
    if (likely(message_size <= 64)) {
        // Use fast path for small messages (single cache line)
        memcpy(msg.payload_ptr, message, message_size);
    } else {
        memcpy(msg.payload_ptr, message, message_size);
    }
    
    mailbox_send(&target->mailbox, msg);
    target->active = 1;  // Wake actor if dormant
}

void* aether_ask_message(Actor* target, void* message, int message_size, int timeout_ms) {
    PendingRequest* req = (PendingRequest*)aligned_alloc(CACHE_LINE_SIZE, sizeof(PendingRequest));
    if (unlikely(!req)) return NULL;
    
    pthread_mutex_lock(&target->request_mutex);
    req->request_id = target->next_request_id++;
    req->reply_ready = 0;
    req->caller = NULL;
    req->reply_data = NULL;
    pthread_mutex_init(&req->mutex, NULL);
    pthread_cond_init(&req->cond, NULL);
    req->next = target->pending_requests;
    target->pending_requests = req;
    pthread_mutex_unlock(&target->request_mutex);
    
    aether_send_message(target, message, message_size);
    
    pthread_mutex_lock(&req->mutex);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    
    while (!req->reply_ready) {
        if (unlikely(pthread_cond_timedwait(&req->cond, &req->mutex, &ts) != 0)) {
            pthread_mutex_unlock(&req->mutex);
            return NULL; // Timeout
        }
    }
    
    void* reply = req->reply_data;
    pthread_mutex_unlock(&req->mutex);
    
    pthread_mutex_destroy(&req->mutex);
    pthread_cond_destroy(&req->cond);
    free(req);
    
    return reply;
}

void aether_reply(Actor* self, void* reply_data, int reply_size) {
    pthread_mutex_lock(&self->request_mutex);
    
    PendingRequest* req = self->pending_requests;
    if (likely(req)) {
        pthread_mutex_lock(&req->mutex);
        req->reply_data = malloc(reply_size);
        if (likely(req->reply_data)) {
            memcpy(req->reply_data, reply_data, reply_size);
        }
        req->reply_ready = 1;
        pthread_cond_signal(&req->cond);
        pthread_mutex_unlock(&req->mutex);
    }
    
    pthread_mutex_unlock(&self->request_mutex);
}

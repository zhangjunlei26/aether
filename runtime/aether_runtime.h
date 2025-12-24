#ifndef AETHER_RUNTIME_H
#define AETHER_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

// Core runtime types
typedef struct ActorRef ActorRef;
typedef struct Message Message;

// Error codes
#define AETHER_SUCCESS 0
#define AETHER_ERROR_OUT_OF_MEMORY 1
#define AETHER_ERROR_INVALID_PARAM 2

// Memory management
void* aether_malloc(size_t size);
void* aether_calloc(size_t count, size_t size);
void* aether_realloc(void* ptr, size_t size);
void aether_free(void* ptr);

// Memory pool management
int aether_memory_init(size_t initial_pool_size);
void aether_memory_cleanup();

#endif // AETHER_RUNTIME_H


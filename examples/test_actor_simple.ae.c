#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aether_runtime.h"

// Actor: Counter (State Machine)
typedef struct Counter {
    int id;
    int active;  // 1 = has messages, 0 = waiting
    Mailbox mailbox;
    
    int count;
} Counter;

void Counter_step(Counter* self) {
    Message msg;
    
    // Try to receive a message
    if (!mailbox_receive(&self->mailbox, &msg)) {
        self->active = 0;  // No messages, yield
        return;
    }
    
    // Process message
count;
(count + 1);
}

int main() {
    aether_runtime_init(4); // Initialize with 4 worker threads
    
    {
printf("Actor compiled!\n");
    }
    
    aether_runtime_shutdown();
    return 0;
}

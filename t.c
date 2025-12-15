#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aether_runtime.h"

// Actor: A (State Machine)
typedef struct A {
    int id;
    int active;  // 1 = has messages, 0 = waiting
    Mailbox mailbox;
    
    int count;
} A;

void A_step(A* self) {
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


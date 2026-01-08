// Test suite for lock-free mailbox implementation
// Tests correctness, performance, and thread safety

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "../../runtime/actors/lockfree_mailbox.h"
#include "../../runtime/actors/actor_state_machine.h"

#define TEST_COUNT 10000
#define THREAD_COUNT 2

// Test 1: Basic send/receive
int test_basic_operations() {
    LockFreeMailbox mbox;
    lockfree_mailbox_init(&mbox);
    
    Message msg = {.type = 1, .sender_id = 0, .payload_int = 42, .payload_ptr = NULL};
    Message recv_msg;
    
    // Test send
    assert(lockfree_mailbox_send(&mbox, msg) == 1);
    
    // Test receive
    assert(lockfree_mailbox_receive(&mbox, &recv_msg) == 1);
    assert(recv_msg.type == 1);
    assert(recv_msg.payload_int == 42);
    
    // Test empty
    assert(lockfree_mailbox_receive(&mbox, &recv_msg) == 0);
    
    printf("✓ Basic operations test passed\n");
    return 1;
}

// Test 2: Fill and drain
int test_capacity() {
    LockFreeMailbox mbox;
    lockfree_mailbox_init(&mbox);
    
    Message msg = {.type = 1, .sender_id = 0, .payload_int = 0, .payload_ptr = NULL};
    
    // Fill to capacity (63 messages, size is 64)
    int sent = 0;
    for (int i = 0; i < 100; i++) {
        msg.payload_int = i;
        if (lockfree_mailbox_send(&mbox, msg)) {
            sent++;
        } else {
            break;
        }
    }
    
    printf("  Capacity: %d messages\n", sent);
    assert(sent == 63);  // One slot always empty in SPSC
    
    // Drain all
    Message recv_msg;
    int received = 0;
    for (int i = 0; i < 100; i++) {
        if (lockfree_mailbox_receive(&mbox, &recv_msg)) {
            assert(recv_msg.payload_int == i);
            received++;
        } else {
            break;
        }
    }
    
    assert(received == sent);
    assert(lockfree_mailbox_is_empty(&mbox));
    
    printf("✓ Capacity test passed\n");
    return 1;
}

// Test 3: Batch operations
int test_batch_operations() {
    LockFreeMailbox mbox;
    lockfree_mailbox_init(&mbox);
    
    Message msgs[16];
    for (int i = 0; i < 16; i++) {
        msgs[i].type = 1;
        msgs[i].payload_int = i;
        msgs[i].payload_ptr = NULL;
    }
    
    // Batch send
    int sent = lockfree_mailbox_send_batch(&mbox, msgs, 16);
    assert(sent == 16);
    
    // Batch receive
    Message recv_msgs[16];
    int received = lockfree_mailbox_receive_batch(&mbox, recv_msgs, 16);
    assert(received == 16);
    
    for (int i = 0; i < 16; i++) {
        assert(recv_msgs[i].payload_int == i);
    }
    
    printf("✓ Batch operations test passed\n");
    return 1;
}

// Test 4: Thread safety (producer/consumer)
typedef struct {
    LockFreeMailbox* mbox;
    int id;
    int* counter;
} ThreadData;

void* producer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    Message msg = {.type = 1, .sender_id = data->id, .payload_int = 0, .payload_ptr = NULL};
    
    for (int i = 0; i < TEST_COUNT / THREAD_COUNT; i++) {
        msg.payload_int = i;
        while (!lockfree_mailbox_send(data->mbox, msg)) {
            // Spin until space available
        }
    }
    
    return NULL;
}

void* consumer_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    Message msg;
    int count = 0;
    
    while (count < TEST_COUNT / THREAD_COUNT) {
        if (lockfree_mailbox_receive(data->mbox, &msg)) {
            (*data->counter)++;
            count++;
        }
    }
    
    return NULL;
}

int test_thread_safety() {
    // SPSC = Single Producer Single Consumer
    // Test with ONE producer and ONE consumer thread
    LockFreeMailbox mbox;
    lockfree_mailbox_init(&mbox);
    
    int counter = 0;
    pthread_t producer;
    pthread_t consumer;
    ThreadData producer_data = {&mbox, 0, &counter};
    ThreadData consumer_data = {&mbox, 1, &counter};
    
    // Start single producer
    pthread_create(&producer, NULL, producer_thread, &producer_data);
    
    // Start single consumer
    pthread_create(&consumer, NULL, consumer_thread, &consumer_data);
    
    // Wait for completion
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);
    
    assert(counter == TEST_COUNT / THREAD_COUNT);
    printf("✓ Thread safety test passed (%d messages)\n", counter);
    return 1;
}

// Test 5: Performance benchmark
int test_performance() {
    LockFreeMailbox mbox;
    lockfree_mailbox_init(&mbox);
    
    Message msg = {.type = 1, .sender_id = 0, .payload_int = 42, .payload_ptr = NULL};
    Message recv_msg;
    
    clock_t start = clock();
    
    for (int i = 0; i < TEST_COUNT; i++) {
        lockfree_mailbox_send(&mbox, msg);
        lockfree_mailbox_receive(&mbox, &recv_msg);
    }
    
    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    double ops_per_sec = TEST_COUNT / seconds;
    
    printf("✓ Performance: %.2f M ops/sec\n", ops_per_sec / 1000000.0);
    return 1;
}

int main() {
    printf("=================================\n");
    printf("  Lock-Free Mailbox Test Suite\n");
    printf("=================================\n\n");
    
    int passed = 0;
    int total = 5;
    
    printf("Running tests...\n\n");
    
    passed += test_basic_operations();
    passed += test_capacity();
    passed += test_batch_operations();
    passed += test_thread_safety();
    passed += test_performance();
    
    printf("\n=================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("=================================\n");
    
    return (passed == total) ? 0 : 1;
}

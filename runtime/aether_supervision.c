#include "aether_supervision.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

SupervisionNode* global_supervision_root = NULL;

// Get current time in seconds
static double get_current_time() {
    return (double)clock() / CLOCKS_PER_SEC;
}

// Supervisor management
SupervisionNode* supervision_create_node(ActorBase* actor, RestartStrategy strategy) {
    SupervisionNode* node = (SupervisionNode*)malloc(sizeof(SupervisionNode));
    node->actor = actor;
    node->strategy = strategy;
    node->max_restarts = 5;  // Default: 5 restarts
    node->restart_count = 0;
    node->restart_window = 60.0;  // Default: 60 seconds
    node->last_restart_time = 0.0;
    node->supervisor = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}

void supervision_free_node(SupervisionNode* node) {
    if (!node) return;
    
    // Free children
    for (int i = 0; i < node->child_count; i++) {
        supervision_free_node(node->children[i]);
    }
    
    if (node->children) {
        free(node->children);
    }
    
    free(node);
}

// Supervision operations
void supervision_add_child(SupervisionNode* supervisor, SupervisionNode* child) {
    if (!supervisor || !child) return;
    
    // Grow children array if needed
    if (supervisor->child_count >= supervisor->child_capacity) {
        int new_capacity = supervisor->child_capacity == 0 ? 4 : supervisor->child_capacity * 2;
        supervisor->children = (SupervisionNode**)realloc(
            supervisor->children,
            new_capacity * sizeof(SupervisionNode*)
        );
        supervisor->child_capacity = new_capacity;
    }
    
    supervisor->children[supervisor->child_count++] = child;
    child->supervisor = supervisor;
}

void supervision_remove_child(SupervisionNode* supervisor, SupervisionNode* child) {
    if (!supervisor || !child) return;
    
    for (int i = 0; i < supervisor->child_count; i++) {
        if (supervisor->children[i] == child) {
            // Shift remaining children
            for (int j = i; j < supervisor->child_count - 1; j++) {
                supervisor->children[j] = supervisor->children[j + 1];
            }
            supervisor->child_count--;
            child->supervisor = NULL;
            break;
        }
    }
}

// Actor crash handling
void supervision_report_crash(SupervisionNode* node, CrashReason reason) {
    if (!node) return;
    
    const char* reason_str = "Unknown";
    switch (reason) {
        case ACTOR_CRASH_PANIC: reason_str = "Panic"; break;
        case ACTOR_CRASH_ASSERT: reason_str = "Assertion failed"; break;
        case ACTOR_CRASH_NULL_DEREF: reason_str = "Null pointer dereference"; break;
        case ACTOR_CRASH_DIV_BY_ZERO: reason_str = "Division by zero"; break;
        case ACTOR_CRASH_STACK_OVERFLOW: reason_str = "Stack overflow"; break;
        case ACTOR_CRASH_OUT_OF_MEMORY: reason_str = "Out of memory"; break;
        default: break;
    }
    
    fprintf(stderr, "[SUPERVISION] Actor %d crashed: %s\n", 
            node->actor ? node->actor->id : -1, reason_str);
    
    // Check if we should restart
    if (supervision_should_restart(node)) {
        fprintf(stderr, "[SUPERVISION] Restarting actor %d (restart count: %d)\n",
                node->actor ? node->actor->id : -1, node->restart_count + 1);
        supervision_restart_actor(node);
    } else {
        fprintf(stderr, "[SUPERVISION] Max restarts exceeded for actor %d, escalating to supervisor\n",
                node->actor ? node->actor->id : -1);
        
        // Escalate to supervisor if exists
        if (node->supervisor) {
            supervision_report_crash(node->supervisor, ACTOR_CRASH_PANIC);
        }
    }
}

int supervision_should_restart(SupervisionNode* node) {
    if (!node) return 0;
    
    double current_time = get_current_time();
    
    // Reset restart count if window has elapsed
    if (current_time - node->last_restart_time > node->restart_window) {
        node->restart_count = 0;
    }
    
    return node->restart_count < node->max_restarts;
}

static RestartCallback restart_callback = NULL;

void supervision_set_restart_callback(SupervisionNode* node, RestartCallback callback) {
    restart_callback = callback;
}

void supervision_restart_actor(SupervisionNode* node) {
    if (!node) return;
    
    node->restart_count++;
    node->last_restart_time = get_current_time();
    
    // Apply restart strategy
    switch (node->strategy) {
        case RESTART_STRATEGY_ONE_FOR_ONE:
            // Restart only this actor
            if (restart_callback) {
                node->actor = restart_callback();
            }
            break;
            
        case RESTART_STRATEGY_ONE_FOR_ALL:
            // Restart this actor and all siblings
            if (node->supervisor) {
                for (int i = 0; i < node->supervisor->child_count; i++) {
                    SupervisionNode* sibling = node->supervisor->children[i];
                    if (restart_callback) {
                        sibling->actor = restart_callback();
                    }
                }
            }
            break;
            
        case RESTART_STRATEGY_REST_FOR_ONE:
            // Restart this actor and those started after it
            if (node->supervisor) {
                int start_restarting = 0;
                for (int i = 0; i < node->supervisor->child_count; i++) {
                    SupervisionNode* sibling = node->supervisor->children[i];
                    if (sibling == node) {
                        start_restarting = 1;
                    }
                    if (start_restarting && restart_callback) {
                        sibling->actor = restart_callback();
                    }
                }
            }
            break;
    }
}

// Global supervision tree
void supervision_init() {
    if (!global_supervision_root) {
        global_supervision_root = supervision_create_node(NULL, RESTART_STRATEGY_ONE_FOR_ONE);
    }
}

void supervision_shutdown() {
    if (global_supervision_root) {
        supervision_free_node(global_supervision_root);
        global_supervision_root = NULL;
    }
}


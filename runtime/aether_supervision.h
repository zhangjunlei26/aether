#ifndef AETHER_SUPERVISION_H
#define AETHER_SUPERVISION_H

#include "actor_state_machine.h"

// Actor supervision types
typedef enum {
    RESTART_STRATEGY_ONE_FOR_ONE,      // Restart only failed actor
    RESTART_STRATEGY_ONE_FOR_ALL,      // Restart all supervised actors
    RESTART_STRATEGY_REST_FOR_ONE      // Restart failed actor and those started after it
} RestartStrategy;

typedef enum {
    ACTOR_CRASH_UNKNOWN = 0,
    ACTOR_CRASH_PANIC,
    ACTOR_CRASH_ASSERT,
    ACTOR_CRASH_NULL_DEREF,
    ACTOR_CRASH_DIV_BY_ZERO,
    ACTOR_CRASH_STACK_OVERFLOW,
    ACTOR_CRASH_OUT_OF_MEMORY
} CrashReason;

// Supervision tree node
typedef struct SupervisionNode {
    ActorBase* actor;
    RestartStrategy strategy;
    int max_restarts;
    int restart_count;
    double restart_window;  // Time window in seconds for restart counting
    double last_restart_time;
    struct SupervisionNode* supervisor;
    struct SupervisionNode** children;
    int child_count;
    int child_capacity;
} SupervisionNode;

// Supervisor management
SupervisionNode* supervision_create_node(ActorBase* actor, RestartStrategy strategy);
void supervision_free_node(SupervisionNode* node);

// Supervision operations
void supervision_add_child(SupervisionNode* supervisor, SupervisionNode* child);
void supervision_remove_child(SupervisionNode* supervisor, SupervisionNode* child);

// Actor crash handling
void supervision_report_crash(SupervisionNode* node, CrashReason reason);
int supervision_should_restart(SupervisionNode* node);
void supervision_restart_actor(SupervisionNode* node);

// Supervision callbacks (implemented by user code)
typedef ActorBase* (*RestartCallback)(void);
void supervision_set_restart_callback(SupervisionNode* node, RestartCallback callback);

// Global supervision tree
extern SupervisionNode* global_supervision_root;

void supervision_init();
void supervision_shutdown();

#endif // AETHER_SUPERVISION_H


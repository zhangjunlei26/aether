#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>

// Aether runtime libraries
#include "actor_state_machine.h"
#include "multicore_scheduler.h"
#include "aether_cpu_detect.h"
#include "aether_string.h"
#include "aether_io.h"
#include "aether_math.h"
#include "aether_supervision.h"
#include "aether_tracing.h"
#include "aether_bounds_check.h"
#include "aether_runtime_types.h"

extern __thread int current_core_id;

typedef struct GameManager {
    int id;
    int active;
    int assigned_core;
    Mailbox mailbox;
    void (*step)(void*);
    
    int secret_number;
    int guesses_made;
    int game_over;
    int won;
} GameManager;

void GameManager_step(GameManager* self) {
    Message msg;
    
    if (!mailbox_receive(&self->mailbox, &msg)) {
        self->active = 0;
        return;
    }
    
    if ((msg.type == 1)) {
        {
self->secret_number = msg.payload_int;
        }
    }
    if ((msg.type == 2)) {
        {
if ((self->game_over == 0)) {
                {
void guess = msg.payload_int;
self->guesses_made = (self->guesses_made + 1);
check_guess(guess);
                }
            }
        }
    }
    if ((msg.type == 3)) {
        {
self->guesses_made = 0;
self->game_over = 0;
self->won = 0;
        }
    }
}

GameManager* spawn_GameManager() {
    GameManager* actor = malloc(sizeof(GameManager));
    actor->id = atomic_fetch_add(&next_actor_id, 1);
    actor->active = 1;
    actor->assigned_core = -1;
    actor->step = (void (*)(void*))GameManager_step;
    mailbox_init(&actor->mailbox);
    actor->secret_number = 42;
    actor->guesses_made = 0;
    actor->game_over = 0;
    actor->won = 0;
    scheduler_register_actor((ActorBase*)actor, -1);
    return actor;
}

void send_GameManager(GameManager* actor, int type, int payload) {
    Message msg = {type, 0, payload, NULL};
    if (actor->assigned_core == current_core_id) {
        scheduler_send_local((ActorBase*)actor, msg);
    } else {
        scheduler_send_remote((ActorBase*)actor, msg, current_core_id);
    }
}

typedef struct ScoreBoard {
    int id;
    int active;
    int assigned_core;
    Mailbox mailbox;
    void (*step)(void*);
    
    int games_played;
    int total_guesses;
    int best_score;
} ScoreBoard;

void ScoreBoard_step(ScoreBoard* self) {
    Message msg;
    
    if (!mailbox_receive(&self->mailbox, &msg)) {
        self->active = 0;
        return;
    }
    
    if ((msg.type == 1)) {
        {
void guesses = msg.payload_int;
self->games_played = (self->games_played + 1);
self->total_guesses = (self->total_guesses + guesses);
if ((guesses < self->best_score)) {
                {
self->best_score = guesses;
                }
            }
        }
    }
    if ((msg.type == 2)) {
        {
printf("%s\n", aether_string_from_literal("\n╔════════════════════════╗\n")->data);
printf("%s\n", aether_string_from_literal("║   Game Statistics      ║\n")->data);
printf("%s\n", aether_string_from_literal("╠════════════════════════╣\n")->data);
printf("%s\n", aether_string_from_literal("║ Games Played: ")->data);
printf("%d\n", self->games_played);
printf("%s\n", aether_string_from_literal("\n")->data);
printf("%s\n", aether_string_from_literal("║ Total Guesses: ")->data);
printf("%d\n", self->total_guesses);
printf("%s\n", aether_string_from_literal("\n")->data);
if ((self->games_played > 0)) {
                {
int avg = (self->total_guesses / self->games_played);
printf("%s\n", aether_string_from_literal("║ Average: ")->data);
printf("%d\n", avg);
printf("%s\n", aether_string_from_literal("\n")->data);
                }
            }
if ((self->best_score < 999)) {
                {
printf("%s\n", aether_string_from_literal("║ Best Score: ")->data);
printf("%d\n", self->best_score);
printf("%s\n", aether_string_from_literal("\n")->data);
                }
            }
printf("%s\n", aether_string_from_literal("╚════════════════════════╝\n")->data);
        }
    }
}

ScoreBoard* spawn_ScoreBoard() {
    ScoreBoard* actor = malloc(sizeof(ScoreBoard));
    actor->id = atomic_fetch_add(&next_actor_id, 1);
    actor->active = 1;
    actor->assigned_core = -1;
    actor->step = (void (*)(void*))ScoreBoard_step;
    mailbox_init(&actor->mailbox);
    actor->games_played = 0;
    actor->total_guesses = 0;
    actor->best_score = 999;
    scheduler_register_actor((ActorBase*)actor, -1);
    return actor;
}

void send_ScoreBoard(ScoreBoard* actor, int type, int payload) {
    Message msg = {type, 0, payload, NULL};
    if (actor->assigned_core == current_core_id) {
        scheduler_send_local((ActorBase*)actor, msg);
    } else {
        scheduler_send_remote((ActorBase*)actor, msg, current_core_id);
    }
}

void play_round(void game, void guess_sequence, void num_guesses) {
    {
printf("%s\n", aether_string_from_literal("\n--- New Round ---\n")->data);
printf("%s\n", aether_string_from_literal("I'm thinking of a number between 1 and 100...\n\n")->data);
for ((i = 0); (i < num_guesses); (i = (i + 1))) {
            {
void guess = guess_sequence[i];
printf("%s\n", aether_string_from_literal("Your guess: ")->data);
printf("[value]\n");
printf("%s\n", aether_string_from_literal("\n")->data);
send_GameManager(game, 2, guess);
GameManager_step(game);
            }
        }
    }
}

int main() {
    // Initialize multi-core actor scheduler
    int num_cores = cpu_recommend_cores();
    MulticoreScheduler* scheduler = scheduler_create(num_cores);
    if (!scheduler) {
        fprintf(stderr, "Failed to create actor scheduler\n");
        return 1;
    }
    scheduler_start(scheduler);
    current_core_id = 0;
    
    {
printf("%s\n", aether_string_from_literal("╔═══════════════════════════════════╗\n")->data);
printf("%s\n", aether_string_from_literal("║  NUMBER GUESSING GAME - Aether    ║\n")->data);
printf("%s\n", aether_string_from_literal("╚═══════════════════════════════════╝\n")->data);
printf("%s\n", aether_string_from_literal("\nWelcome! I'll pick a number, you try to guess it!\n")->data);
GameManager* game = spawn_GameManager();
ScoreBoard* scoreboard = spawn_ScoreBoard();
send_GameManager(game, 1, 42);
GameManager_step(game);
int guesses1[5] = {50, 30, 40, 45, 42};
play_round(game, guesses1, 5);
send_ScoreBoard(scoreboard, 1, 5);
ScoreBoard_step(scoreboard);
send_GameManager(game, 3, 0);
GameManager_step(game);
send_GameManager(game, 1, 75);
GameManager_step(game);
int guesses2[2] = {50, 75};
play_round(game, guesses2, 2);
send_ScoreBoard(scoreboard, 1, 2);
ScoreBoard_step(scoreboard);
send_GameManager(game, 3, 0);
GameManager_step(game);
send_GameManager(game, 1, 10);
GameManager_step(game);
int guesses3[5] = {50, 25, 12, 8, 10};
play_round(game, guesses3, 5);
send_ScoreBoard(scoreboard, 1, 5);
ScoreBoard_step(scoreboard);
printf("%s\n", aether_string_from_literal("\n=== Final Statistics ===\n")->data);
send_ScoreBoard(scoreboard, 2, 0);
ScoreBoard_step(scoreboard);
printf("%s\n", aether_string_from_literal("\nThanks for playing! 🎮\n")->data);
    }
    
    // Wait for actors to complete and clean up
    scheduler_join(scheduler);
    scheduler_destroy(scheduler);
    return 0;
}

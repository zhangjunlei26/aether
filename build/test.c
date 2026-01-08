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

int main() {
    {
printf("%s\n", aether_string_from_literal("Hello from Aether!\n")->data);
printf("%s\n", aether_string_from_literal("Testing the new build system.\n")->data);
int result = 42;
printf("%s\n", aether_string_from_literal("The answer is: %d\n")->data);
printf("%d\n", result);
    }
    return 0;
}

#include <stdio.h>

#include "../../std/log/aether_log.h"

void print_arr(char *pargv[], int n) {
    for (short i = 0; i < n; i++) {
        log_info("argv%d:%s", i, pargv[i]);
    }
}

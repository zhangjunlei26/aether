#include <stdio.h>

#include "../../std/log/aether_log.h"

enum { LOG_TRACE, LOG_DEBUG };

const char **test; //  test install

void print_arr(char *pargv[], int n) {
    for (short i = 0; i < n; i++) {
        LOG_INFO("argv%d:%s", i, pargv[i]);
    }
}

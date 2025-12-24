#include "test_harness.h"
#include <string.h>

int main(int argc, char** argv) {
    printf("Aether Language Test Suite\n");
    printf("==========================\n\n");
    
    if (argc > 1) {
        printf("Note: Test filtering not yet implemented. Running all tests.\n\n");
    }
    
    run_all_tests();
    
    return 0;
}

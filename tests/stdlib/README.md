# Aether Standard Library Test Suite

Professional test suite for Aether's standard library collections.

## Overview

This test suite provides comprehensive coverage for:
- **Vector** (9 tests) - Dynamic array implementation
- **HashMap** (10 tests) - Robin Hood hash table
- **Set** (6 tests) - Hash-based set operations

**Total: 25 tests**

## Features

- **Cross-platform** - Works on Windows, Linux, macOS
- **Professional structure** - Common test framework
- **Clear output** - Colored pass/fail indicators
- **No redundancy** - Shared utilities and helpers
- **Easy to run** - Single command execution
- **Comprehensive** - Tests creation, operations, edge cases, memory management

## Quick Start

### Windows (PowerShell)
```powershell
cd tests/stdlib
.\run_tests.ps1
```

### Linux/macOS (Bash)
```bash
cd tests/stdlib
chmod +x run_tests.sh
./run_tests.sh
```

## Test Framework

All tests use a common framework (`test_framework.h`) that provides:

### Macros
- `TEST_SUITE_BEGIN(name)` - Initialize test suite
- `TEST(name, condition)` - Run a test assertion
- `TEST_MSG(name, condition, msg)` - Test with custom error message
- `TEST_SUITE_END()` - Print results and return exit code

### Helper Functions
- `test_int_equals()` - Integer comparison
- `test_int_hash()` - Integer hash function
- `test_int_clone()` - Integer clone function
- `test_str_hash()` - String hash (djb2 algorithm)
- `test_str_equals()` - String comparison
- `test_make_int(value)` - Create heap-allocated integer

## Test Structure

Each test file follows a consistent pattern:

```c
#include "test_framework.h"
#include "../../std/collections/aether_vector.h"

int main(void) {
    TEST_SUITE_BEGIN("Vector Tests");
    
    // Setup
    Vector* v = vector_create(10, free, test_int_clone);
    
    // Tests
    TEST("Create", v != NULL && vector_size(v) == 0);
    TEST("Push", vector_size(v) == 3);
    
    // Cleanup
    vector_free(v);
    
    TEST_SUITE_END();
}
```

## Output Format

```
===================================
  Aether Standard Library Tests
===================================

Compiling Vector tests...
Running Vector tests...

=== Vector Tests ===

Create... OK
Push... OK
Get... OK
...

====================
All 9 tests passed!
====================

===================================
  Test Summary
===================================
Passed: 3
Failed: 0
===================================

All tests passed!
```

## Test Coverage

| Module | Tests | Lines | Coverage |
|--------|-------|-------|----------|
| Vector | 9 | ~200 | Create, push, pop, get, set, insert, remove, find, clear, auto-grow |
| HashMap | 10 | ~250 | Create, insert, get, contains, update, remove, clear, collisions, resize |
| Set | 6 | ~150 | Create, add, contains, remove, union, intersection |

## Adding New Tests

1. Create test file in `tests/stdlib/`:
```c
#include "test_framework.h"
#include "../../std/collections/your_module.h"

int main(void) {
    TEST_SUITE_BEGIN("YourModule Tests");
    
    // Your tests here
    TEST("Test name", condition);
    
    TEST_SUITE_END();
}
```

2. Update test runners:
   - Add compilation and execution in `run_tests.ps1`
   - Add compilation and execution in `run_tests.sh`

## Compiler Flags

Both test runners use consistent flags:
- `-I.` - Project root for includes
- `-Istd` - Standard library headers
- `-Iruntime` - Runtime headers
- `-Itests/stdlib` - Test framework header
- `-std=c11` - C11 standard
- `-Wall -Wextra` - All warnings enabled

## Known Issues

None currently.

## Requirements

- **Compiler**: GCC or Clang with C11 support
- **OS**: Windows (PowerShell), Linux (Bash), macOS (Bash)
- **Dependencies**: Aether runtime and standard library

## CI/CD Integration

The test suite returns proper exit codes:
- `0` - All tests passed
- `1` - One or more tests failed

Example GitHub Actions:
```yaml
- name: Run stdlib tests
  run: |
    cd tests/stdlib
    ./run_tests.sh
```

## Maintenance

The test suite is designed for easy maintenance:
- **Single source of truth**: `test_framework.h` contains all common utilities
- **Consistent patterns**: All tests follow the same structure
- **Clear naming**: Test names describe what they verify
- **Automatic cleanup**: Test runners remove compiled binaries

## Contributing

When adding new tests:
1. Follow the existing pattern
2. Use the test framework macros
3. Add clear test names
4. Include setup and cleanup
5. Update both test runners
6. Update this README

## License

Part of the Aether programming language project.

# Aether Runtime Library Guide

## Overview

Aether provides a comprehensive runtime library that integrates seamlessly with the language. All runtime functions are automatically available in your Aether programs.

## String Library (`aether_string.h`)

### Types

```c
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} AetherString;
```

### Functions

#### Creating Strings

```aether
// From literal (automatic in Aether)
let greeting = "Hello, World!";

// From C functions (when interop needed)
aether_string_new(size)           // Create empty string with capacity
aether_string_from_literal(str)   // Create from C string literal
```

#### String Operations

```aether
aether_string_concat(s1, s2)       // Concatenate two strings
aether_string_length(str)          // Get string length
aether_string_get_char(str, index) // Get character at index
aether_string_compare(s1, s2)      // Compare strings (-1, 0, 1)
aether_string_equals(s1, s2)       // Check equality
```

#### String Searching

```aether
aether_string_contains(haystack, needle)     // Check if contains
aether_string_find(haystack, needle)         // Find first occurrence
aether_string_replace_all(str, old, new)     // Replace all occurrences
```

#### String Transformations

```aether
aether_string_to_upper(str)        // Convert to uppercase
aether_string_to_lower(str)        // Convert to lowercase
aether_string_trim(str)            // Trim whitespace
aether_string_substring(str, start, len)  // Extract substring
```

#### Memory Management

```aether
aether_string_free(str)            // Free string memory
aether_string_to_cstr(str)         // Convert to C string (must free)
```

**Note**: Aether strings are reference-counted and automatically managed in most cases.

---

## I/O Library (`aether_io.h`)

### Console Output

```aether
aether_print(value)                // Print without newline
aether_print_line(str)             // Print string with newline
aether_print_int(n)                // Print integer
aether_print_float(f)              // Print float
```

### Console Input

```aether
aether_read_line()                 // Read line from stdin
```

### File Operations

```aether
// Read entire file into string
let content = aether_read_file("data.txt");

// Write string to file
aether_write_file("output.txt", content);

// Append to file
aether_append_file("log.txt", "Log entry\n");

// Check if file exists
if (aether_file_exists("config.json")) {
    // ...
}

// Delete file
aether_delete_file("temp.txt");
```

### File Information

```aether
typedef struct {
    size_t size;
    int is_directory;
    long modified_time;
} AetherFileInfo;

let info = aether_file_info("data.txt");
aether_print_int(info.size);
```

---

## Math Library (`aether_math.h`)

### Basic Operations

```aether
aether_abs_int(x)                  // Absolute value (int)
aether_abs_float(x)                // Absolute value (float)
aether_min_int(a, b)               // Minimum (int)
aether_max_int(a, b)               // Maximum (int)
aether_min_float(a, b)             // Minimum (float)
aether_max_float(a, b)             // Maximum (float)
```

### Advanced Math

```aether
aether_sqrt(x)                     // Square root
aether_pow(base, exp)              // Power (base^exp)
aether_floor(x)                    // Floor
aether_ceil(x)                     // Ceiling
aether_round(x)                    // Round to nearest
```

### Trigonometry

```aether
aether_sin(x)                      // Sine
aether_cos(x)                      // Cosine
aether_tan(x)                      // Tangent
aether_asin(x)                     // Arc sine
aether_acos(x)                     // Arc cosine
aether_atan(x)                     // Arc tangent
aether_atan2(y, x)                 // Arc tangent (2 args)
```

### Random Numbers

```aether
aether_random_seed(seed)           // Set random seed
aether_random_int(min, max)        // Random int in range
aether_random_float(min, max)      // Random float in range
```

### Constants

```aether
AETHER_PI                          // 3.14159...
AETHER_E                           // 2.71828...
```

### Example

```aether
main() {
    // Calculate distance
    let dx = 3.0;
    let dy = 4.0;
    let distance = aether_sqrt(dx * dx + dy * dy);
    aether_print_float(distance);  // 5.0
    
    // Random number
    aether_random_seed(42);
    let roll = aether_random_int(1, 6);  // Dice roll
    aether_print_int(roll);
}
```

---

## Actor Supervision (`aether_supervision.h`)

### Creating Supervision Trees

```aether
supervision_init();

// Create supervisor
let supervisor = supervision_create_node(
    NULL,                          // No actor (root supervisor)
    RESTART_STRATEGY_ONE_FOR_ONE   // Restart strategy
);

// Create supervised actor
let worker = spawn_worker();
let worker_node = supervision_create_node(
    worker,
    RESTART_STRATEGY_ONE_FOR_ONE
);

// Add to supervision tree
supervision_add_child(supervisor, worker_node);
```

### Restart Strategies

- `RESTART_STRATEGY_ONE_FOR_ONE` - Restart only crashed actor
- `RESTART_STRATEGY_ONE_FOR_ALL` - Restart all children
- `RESTART_STRATEGY_REST_FOR_ONE` - Restart crashed actor and all after it

### Handling Crashes

```aether
// Actors crash → Supervisor automatically restarts them
// Too many crashes → Escalates to parent supervisor
```

---

## Message Tracing (`aether_tracing.h`)

### Enable Tracing

```aether
main() {
    // Initialize tracing with log file
    tracing_init("app.log");
    tracing_enable();
    
    // Trace specific actor
    let worker = spawn_worker();
    tracing_add_actor(worker.id);
    
    // Or trace all actors
    tracing_set_trace_all(1);
}
```

### Output Format

```
[T+0.000ms] actor_1: received msg{type=1, payload=42}
[T+0.002ms] actor_1: sent msg{type=2} to actor_2
[T+0.005ms] actor_2: processed msg{type=2, payload=0}
```

### Functions

```aether
tracing_init(filename)             // Initialize with log file
tracing_enable()                   // Enable tracing
tracing_disable()                  // Disable tracing
tracing_add_actor(id)              // Trace specific actor
tracing_remove_actor(id)           // Stop tracing actor
tracing_set_trace_all(enabled)     // Trace all actors
tracing_log_custom(id, event)      // Log custom event
```

---

## Bounds Checking (`aether_bounds_check.h`)

### Compile-Time Control

```bash
# Debug mode (bounds checking enabled)
gcc -DAETHER_DEBUG output.c -o program_debug

# Release mode (no overhead)
gcc -DAETHER_RELEASE output.c -o program_release
```

### Features

- Array bounds checking
- Null pointer checking
- Division by zero checking
- Custom assertions

### Error Output

```
main.c:42: error: Array index out of bounds
  Array: buffer
  Index: 100
  Length: 10
  Valid range: [0, 10)
```

### Manual Assertions

```aether
// In Aether code (when bounds checking enabled)
AETHER_ASSERT(x > 0, "x must be positive");
```

---

## Testing Framework (`aether_test.h`)

### Writing Tests

```aether
test "counter increments" {
    let c = spawn_counter();
    send_counter(c, 1, 0);
    assert_eq(c.count, 1);
}

test "math operations" {
    assert_eq(add(2, 3), 5);
    assert_ne(subtract(5, 2), 2);
    assert_true(10 > 5);
    assert_false(5 > 10);
}

test "string operations" {
    let s = "hello";
    assert_str_eq(s, "hello");
    assert_not_null(s);
}
```

### Assertion Macros

- `assert_true(condition)`
- `assert_false(condition)`
- `assert_eq(a, b)`
- `assert_ne(a, b)`
- `assert_lt(a, b)` / `assert_le(a, b)`
- `assert_gt(a, b)` / `assert_ge(a, b)`
- `assert_null(ptr)` / `assert_not_null(ptr)`
- `assert_str_eq(s1, s2)`
- `assert_actor_alive(actor)`

### Running Tests

```bash
# Compile with test framework
gcc your_tests.c -Iruntime runtime/aether_test.c -o tests

# Run
./tests

# Output:
# === Running Aether Tests ===
# Test 1/3: counter increments ... PASS
# Test 2/3: math operations ... PASS
# Test 3/3: string operations ... PASS
# 
# === Test Summary ===
# Total: 3
# Passed: 3
# Failed: 0
# 
# ✓ All tests passed!
```

---

## Memory Management

### Automatic

- Strings are reference-counted
- Actors managed by runtime
- Most allocations handled automatically

### Manual

```aether
// Dynamic arrays
let buffer = make([]int, 1000);
// ... use buffer ...
free(buffer);  // Manual free when done

// Custom allocations (when interop with C)
let ptr = malloc(size);
// ... use ptr ...
free(ptr);
```

---

## Best Practices

1. **Use runtime functions** - They're optimized and safe
2. **Enable bounds checking in debug** - Catch bugs early
3. **Use tracing for debugging** - Better than print statements
4. **Supervise critical actors** - Fault tolerance is key
5. **Free manual allocations** - Prevent leaks
6. **Write tests** - Use built-in framework
7. **Disable bounds checking in production** - Zero overhead

---

## Example: Complete Program

```aether
struct Player {
    name: string,
    health: int,
    score: int
}

actor game_server {
    state players = make([]Player, 100)
    state count = 0
    
    receive(msg) {
        if (msg.type == 1) {  // Add player
            players[count] = Player{
                name: "Player" + aether_string_from_int(count),
                health: 100,
                score: 0
            };
            count++;
            
            aether_print_line("Player joined: " + players[count-1].name);
            tracing_log_custom(self.id, "player_joined");
        }
    }
}

main() {
    // Setup
    tracing_init("game.log");
    tracing_enable();
    supervision_init();
    
    // Create supervised server
    let server = spawn_game_server();
    let supervisor = supervision_create_node(NULL, RESTART_STRATEGY_ONE_FOR_ONE);
    let server_node = supervision_create_node(server, RESTART_STRATEGY_ONE_FOR_ONE);
    supervision_add_child(supervisor, server_node);
    
    // Add players
    for (let i = 0; i < 10; i++) {
        send_game_server(server, 1, 0);
    }
    
    aether_print_line("Game server running!");
}
```

This demonstrates: strings, structs, actors, supervision, tracing, arrays, and I/O - all working together.


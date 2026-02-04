# Aether Language Reference

Complete syntax and semantics of the Aether programming language.

## Overview

Aether is a statically-typed, compiled language combining Erlang-inspired actor concurrency with type inference. It features clean, minimal syntax and compiles to C code.

## Types

### Primitive Types

- `int` - 32-bit signed integer
- `float` - 64-bit floating point
- `string` - UTF-8 encoded strings
- `bool` - Boolean type
- `void` - No value (for functions that don't return)

### User-Defined Types

- `struct` - Composite data type
- `actor` - Concurrency primitive with state and message handling
- `array` - Fixed-size homogeneous collections

## Variables

Variables support both explicit types and automatic type inference:

```aether
// Type inference (recommended)
x = 10
y = 20
name = "Alice"

// Explicit types (optional)
int z = 30
string greeting = "Hello"
```

Variables are inferred from their initialization or usage context.

## Functions

Functions support type inference for parameters and return types:

```aether
// Type inference (recommended)
add(a, b) {
    return a + b
}

greet(name) {
    print("Hello, ")
    print(name)
    print("\n")
}

// Explicit types (optional, for clarity)
int add_explicit(int a, int b) {
    return a + b
}

void print_hello() {
    print("Hello\n")
}
```

Functions can return values or `void`. The `main()` function is the entry point.
Types are inferred from usage when not explicitly specified.

### Pattern Matching Functions

Functions support Erlang-style pattern matching with guards:

```aether
fib(0) -> 1
fib(1) -> 1
fib(n) when n > 1 -> fib(n-1) + fib(n-2)

classify(x) when x < 0 -> "negative"
classify(x) when x == 0 -> "zero"
classify(x) when x > 0 -> "positive"
```

## Control Flow

### If Statements

```aether
if (x > 0) {
    print("Positive\n")
} else {
    print("Non-positive\n")
}
```

### While Loops

```aether
i = 0
while (i < 10) {
    print(i)
    print("\n")
    i = i + 1
}
```

### For Loops

```aether
for (i = 0; i < 10; i = i + 1) {
    print(i)
    print("\n")
}
```

### Match Expressions

```aether
result = match (value) {
    0 => "zero"
    1 => "one"
    _ => "other"
}
```

## Structs

```aether
struct Point {
    x,
    y
}

main() {
    p = Point{ x: 10, y: 20 }
    print(p.x)
    print(p.y)
}
```

Structs group related data. Fields are accessed with the `.` operator.
Struct literals use the `StructName{ field: value }` syntax.
Field types are inferred from usage. Explicit types are also supported:

```aether
struct Point {
    int x
    int y
}
```

## Messages

Messages define structured data for actor communication:

```aether
message Increment {
    amount: int
}

message Greet {
    name: string
}

message Reset {}
```

Each message type has named fields with explicit types. Empty messages are also valid.

## Actors

Actors are the core concurrency primitive. They encapsulate state and process messages through pattern matching.

### Actor Definition

```aether
message Increment { amount: int }
message GetCount { dummy: int }

actor Counter {
    state count = 0

    receive {
        Increment(amount) -> {
            count = count + amount
        }
        GetCount() -> {
            print(count)
            print("\n")
        }
        _ -> {
            print("Unknown message\n")
        }
    }
}
```

### Actor State

State variables are declared with the `state` keyword and persist across messages:

```aether
actor Counter {
    state count = 0
    state total = 0
}
```

### Message Handling

The `receive` block uses pattern matching to dispatch incoming messages by type. Each clause destructures the message fields and executes the corresponding handler. The wildcard pattern `_` handles unrecognized messages.

### Spawning Actors

```aether
c = spawn(Counter())
```

The `spawn(ActorName())` syntax creates a new actor instance. The compiler generates the underlying spawn function automatically.

### Sending Messages

```aether
// Fire-and-forget
c ! Increment { amount: 1 }

// Request-reply
result = c ? GetCount { dummy: 0 }
```

The `!` operator sends a message asynchronously. The `?` operator sends a message and waits for a reply.

## Expressions

### Arithmetic

```aether
sum = a + b
diff = a - b
prod = a * b
quot = a / b
```

### Comparison

```aether
if (a == b) { }
if (a != b) { }
if (a < b) { }
if (a > b) { }
if (a <= b) { }
if (a >= b) { }
```

### Postfix Operators

```aether
i++
i--
```

### Member Access

```aether
point.x = 10
```

## Built-in Functions

- `print(format, ...)` - Print formatted string (similar to printf)

## Comments

```aether
// Single-line comment

/* Multi-line
   comment */
```

## Compilation

Aether programs compile to C code, which is then compiled to native executables.

```bash
aetherc program.ae output.c
gcc output.c -Iruntime -o program
```

## Type System

Aether uses static typing with type inference. Types are deduced automatically from initialization and usage context, and checked at compile time. Explicit type annotations are optional and can be used where additional clarity is desired.

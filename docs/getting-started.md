# Getting Started with Aether

This guide covers installation and basic usage of the Aether programming language.

## Installation

### Prerequisites

**All Platforms:**
- Git
- GCC 9.0+ or Clang 10.0+
- Make
- pthread support (usually built-in)

**Windows:**
- MinGW-w64 GCC 11.0+ (MSVC is not supported)
- PowerShell 5.1+

**macOS:**
- Xcode command line tools: `xcode-select --install`
- Homebrew GCC recommended: `brew install gcc`

**Linux:**
- `sudo apt-get install build-essential` (Debian/Ubuntu)

### Building from Source

```bash
git clone https://github.com/nicolasmd87/aether.git
cd aether
make ae
```

This builds the compiler and the `ae` CLI tool. Verify:

```bash
./build/ae version
```

**Windows (MinGW):**
```bash
mingw32-make ae
```

## Your First Aether Program

**Option A: Create a project (recommended)**

```bash
./build/ae init myproject
cd myproject
../build/ae run
```

This creates a project with `aether.toml`, `src/main.ae`, and `tests/`.

**Option B: Run a single file**

Create `hello.ae`:

```aether
main() {
    print("Hello, Aether!\n");
}
```

```bash
./build/ae run hello.ae
```

**Build an executable:**

```bash
./build/ae build hello.ae -o hello
./hello
```

## Actor-Based Programming

Aether is built around the actor model. Here is a simple counter example:

```aether
message Ping {}

actor counter {
    state count = 0

    receive {
        Ping() -> {
            count = count + 1
            print(count)
        }
    }
}

main() {
    c = spawn(counter())
    c ! Ping {}
}
```

Actors are lightweight concurrent entities that communicate through asynchronous messages. Each actor has private state and a mailbox for incoming messages. Messages are defined with the `message` keyword and sent with the `!` operator. The runtime distributes actors across available CPU cores automatically.

## Module System

Import modules using the `import` statement:

```aether
import std.collections.HashMap
import std.log as Log

main() {
    Log.info("Starting application")

    map = HashMap.new()
    map.insert("key", "value")

    print(map.get("key"))
}
```

## Standard Library

Aether includes a standard library with the following modules:

### Collections
- **HashMap**: Hash map with Robin Hood hashing
- **Set**: Set operations (union, intersection, difference)
- **Vector**: Dynamic array with amortized O(1) append
- **PriorityQueue**: Binary heap for priority-based scheduling

### Utilities
- **log**: Structured logging with levels
- **fs**: File system operations
- **net**: Networking utilities

See [stdlib-reference.md](stdlib-reference.md) for the full API reference.

## Pattern Matching

Aether supports pattern matching in match statements:

```aether
match (value) {
    0 -> { print("Zero\n") }
    1 -> { print("One\n") }
    _ -> { print("Other\n") }
}
```

List patterns work with arrays (requires corresponding `_len` variable):

```aether
nums = [1, 2, 3]
nums_len = 3

match (nums) {
    [] -> { print("empty\n") }
    [x] -> { print("one element\n") }
    [h|t] -> {
        print("head: ")
        print(h)
        print("\n")
    }
}
```

## Next Steps

- Read the [Tutorial](tutorial.md) for a guided introduction
- Explore [Standard Library Documentation](stdlib-reference.md)
- See [Architecture](architecture.md) for compiler and runtime internals
- See [Runtime Optimizations](runtime-optimizations.md) for performance details

## Troubleshooting

### Build Failures

**"gcc: command not found" (Windows)**
- Ensure MinGW bin directory is in PATH
- Verify: `gcc --version`

**"pthread.h: No such file or directory"**
- Linux: `sudo apt-get install libpthread-stubs0-dev`
- MinGW includes pthread by default

**Test failures**
- Run specific test category: `./build/test_runner --category=compiler`
- Check for port conflicts if network tests fail (port 8080)

### Common Pitfalls

1. Forgetting to rebuild after changes: run `make clean && make`
2. Using MSVC on Windows: Aether requires GCC (MinGW)
3. Missing `-lpthread` flag when compiling manually
4. Actor structs missing the `migrate_to` field (causes struct layout mismatch)

### Platform-Specific Notes

**macOS:**
- May need `xcode-select --install` for command line tools
- Homebrew GCC recommended: `brew install gcc`

**Linux:**
- Kernel 4.14+ recommended for full NUMA support
- AddressSanitizer may require `gcc-multilib` on some distributions

**Windows:**
- Use PowerShell, not CMD
- Forward slashes in paths work: `./build/ae`

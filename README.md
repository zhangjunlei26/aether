# Aether Programming Language

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)]()

A high-performance, actor-based systems programming language designed for modern multi-core systems. Aether combines the simplicity of high-level languages with the performance of C, featuring lightweight actors, gradual typing, and zero-cost C interoperability.

## Overview

Aether is a compiled language that brings Erlang-style actor concurrency to systems programming. Built for performance-critical applications requiring high throughput and low latency, Aether compiles directly to C for maximum portability and zero runtime overhead.

**What makes Aether different:**
- Lock-free actor mailboxes with 1.8x speedup under multi-core contention
- Gradual type system combining type inference with optional annotations
- Zero-copy message passing between actors
- Sub-microsecond actor spawn latency
- Compiles to readable, portable C code

## Performance

Real-world benchmark results on Intel i7-13700K (4 cores):

| Metric | Performance |
|--------|-------------|
| Message throughput (lock-free) | 2,764 M msg/sec |
| Message throughput (simple) | 1,536 M msg/sec |
| Multi-core speedup | 1.8x |
| Actor memory footprint | 128 bytes |

See [experiments/concurrency/RESULTS.md](experiments/concurrency/RESULTS.md) for detailed benchmarks.

## Key Features

### Concurrency & Performance
- **Lock-Free Mailboxes**: SPSC atomic queues with cache-line alignment
- **Auto-Tuning Runtime**: CPU feature detection (AVX2, MWAIT, SSE4.2)
- **Zero-Sharing Design**: Partitioned scheduler eliminates contention
- **Adaptive Idle**: Multi-phase idle strategy (spin → pause → MWAIT → sleep)

### Type System
- **Gradual Typing**: Write untyped code, add types where needed
- **Hindley-Milner Inference**: Automatic type deduction
- **Optional Annotations**: Explicit types for documentation and safety

### Memory & Safety
- **Arena Allocation**: Automatic lifetime management
- **Bounds Checking**: Optional runtime verification
- **Thread-Local Pools**: Zero-mutex message allocation

### Interoperability
- **C Compilation**: Outputs readable, portable C99 code
- **Zero-Cost FFI**: Direct C function calls with no overhead
- **C Header Import**: Use existing C libraries seamlessly

## Quick Start

### Prerequisites

**Required:**
- GCC 11+ or Clang 14+
- Make (or mingw32-make on Windows)
- Git

**Optional:**
- Docker Desktop (recommended for consistent environment)

### Installation

```bash
git clone https://github.com/nicolasmd87/aether.git
cd aether

# Build compiler and CLI tool
make compiler
make ae

# Test it works
./build/ae version
# Aether 0.4.0
```

**Windows:** Use `mingw32-make` instead of `make`.

### Your First Program

Create `hello.ae`:

```aether
main() {
    print("Hello from Aether!\\n");
    print("The answer is: %d\\n", 42);
}
```

Run it:

```bash
./build/ae run hello.ae
```

That's it! No configuration, no build files needed.

**Docker (Recommended for Windows):**

```bash
# Build container
docker build -t aether:latest -f docker/Dockerfile .

# Start development shell
docker run -it -v $(pwd):/aether aether:latest

# Inside container
make compiler
make test
```

See [docker/README.md](docker/README.md) for detailed setup.

### Running Your First Program

Create `hello.ae`:

```aether
main() {
    print("Hello from Aether!\n");
    print("The answer is: %d\n", 42);
}
```

**Go-style commands (recommended):**

```bash
# Run directly
./build/ae run hello.ae

# Or build to executable
./build/ae build hello.ae -o hello
./build/hello
```

**Using Make:**

```bash
# Run
make run FILE=hello.ae

# Build
make compile FILE=hello.ae OUTPUT=hello
```

**Manual (advanced):**

```bash
# 1. Compile .ae to .c
./build/aetherc hello.ae hello.c

# 2. Compile C to executable
gcc hello.c -o hello

# 3. Run
./hello
```

### Common Commands

**Using ae (Go-style):**

```bash
./build/ae run hello.ae          # Run a program
./build/ae build app.ae -o myapp # Build executable
./build/ae compile lib.ae        # Compile to C only
./build/ae test                  # Run tests
./build/ae help                  # Show help
```

**Using Make:**

```bash
make compiler                    # Build compiler
make ae                          # Build ae CLI tool
make run FILE=hello.ae           # Run a program
make compile FILE=app.ae         # Build executable
make test                        # Run tests
make -j8                         # Parallel build
make help                        # Show all commands
```

**Windows:** Use `mingw32-make` instead of `make` and `.\build\ae.exe` instead of `./build/ae`.

### Adding `ae` to PATH

**Linux/macOS:**
```bash
# After building
sudo cp build/ae /usr/local/bin/
# Or add to ~/.bashrc:
export PATH="$PATH:/path/to/aether/build"

# Now use anywhere:
ae run myapp.ae
```

**Windows:**
```powershell
# Add to PATH via System Properties > Environment Variables
# Or use full path:
D:\Git\aether\build\ae.exe run myapp.ae
```

## Build System Philosophy

Aether provides two interfaces:

**1. `ae` command (Go-style) - Recommended**

```bash
./build/ae run file.ae           # Like: go run, cargo run, zig run
./build/ae build file.ae         # Like: go build, cargo build, zig build
```

Simple, fast, no configuration needed.

**2. Make (traditional) - For advanced use**

```bash
make compiler                    # Build the compiler
make run FILE=file.ae           # Run with Make
make test                       # Run tests
```

**Comparison with other languages:**

| Language | Tool | Run Command | Build Command |
|----------|------|-------------|---------------|
| **Aether** | ae | `ae run app.ae` | `ae build app.ae` |
| Go | go | `go run app.go` | `go build app.go` |
| Rust | cargo | `cargo run` | `cargo build` |
| Zig | zig | `zig run app.zig` | `zig build` |
| C/C++ | Make | Manual | `make` |

**Why both?**
- `ae` command: Simple, fast for daily development
- Make: Full control, parallel builds, CI/CD integration

## Project Structure

```
aether/
├── compiler/           # Aether compiler (lexer, parser, codegen)
│   ├── lexer.c/h      # Tokenization
│   ├── parser.c/h     # AST generation
│   ├── typechecker.c/h # Type inference and checking
│   ├── codegen.c/h    # C code generation
│   └── aetherc.c      # Compiler entry point
├── runtime/           # Runtime system
│   ├── actors/        # Actor implementation and scheduling
│   ├── memory/        # Arena allocators and memory pools
│   ├── scheduler/     # Multi-core work-stealing scheduler
│   └── utils/         # CPU detection, SIMD, tracing
├── std/               # Standard library
│   ├── collections/   # HashMap, Vector, List
│   ├── io/           # File I/O, streams
│   ├── net/          # TCP/UDP networking
│   └── json/         # JSON parser
├── tests/            # Test suite
│   ├── compiler/     # Compiler unit tests
│   ├── runtime/      # Runtime tests
│   └── integration/  # End-to-end tests
├── examples/         # Example programs
│   ├── basic/        # Hello world, simple actors
│   ├── language-features/ # Type system, syntax
│   └── real-world/   # Web servers, chat apps
├── experiments/      # Performance experiments
│   └── concurrency/  # Lock-free optimizations, benchmarks
├── docs/            # Documentation
│   ├── getting-started.md
│   ├── language-reference.md
│   ├── tutorial.md
│   └── architecture.md
└── docker/          # Docker configuration
```

## Language Example

```aether
// Distributed counter with actor supervision
actor Counter {
    var count = 0
    
    receive {
        Increment => count += 1
        
        Decrement => count -= 1
        
        GetCount(reply) => {
            reply.send(count)
        }
        
        Reset => count = 0
    }
}

actor Supervisor {
    var counters: [ActorRef<Counter>] = []
    
    init() {
        // Spawn 4 counter actors
        for i in 0..4 {
            counters.push(spawn Counter)
        }
    }
    
    receive {
        IncrementAll => {
            for counter in counters {
                counter ! Increment
            }
        }
        
        GetTotal(reply) => {
            var total = 0
            for counter in counters {
                let count = counter !? GetCount
                total += count
            }
            reply.send(total)
        }
    }
}

// Main program
let supervisor = spawn Supervisor
supervisor ! IncrementAll
let total = supervisor !? GetTotal
println("Total count: {total}")
```

## Advanced Features

### Runtime Configuration

Configure runtime optimizations at startup:

```c
#include "runtime/aether_runtime.h"

int main() {
    // Auto-detect CPU features and enable optimizations
    aether_runtime_init(4, AETHER_FLAG_AUTO_DETECT);
    
    // Or manually configure
    aether_runtime_init(4, 
        AETHER_FLAG_LOCKFREE_MAILBOX |
        AETHER_FLAG_LOCKFREE_POOLS |
        AETHER_FLAG_ENABLE_MWAIT |
        AETHER_FLAG_VERBOSE
    );
    
    // Your actor system runs here
    
    return 0;
}
```

Available flags:
- `AETHER_FLAG_AUTO_DETECT` - Detect CPU features and enable optimizations
- `AETHER_FLAG_LOCKFREE_MAILBOX` - Use lock-free SPSC mailboxes
- `AETHER_FLAG_LOCKFREE_POOLS` - Thread-local message pools
- `AETHER_FLAG_ENABLE_SIMD` - AVX2 vectorization
- `AETHER_FLAG_ENABLE_MWAIT` - MWAIT-based idle (x86 only)
- `AETHER_FLAG_VERBOSE` - Print runtime configuration

### Gradual Typing

Start with dynamic typing, add types incrementally:

```aether
// Fully dynamic
actor Database {
    var data = {}
    
    receive {
        Store(key, value) => data[key] = value
        Get(key, reply) => reply.send(data[key])
    }
}

// Add types for safety
actor Database {
    var data: HashMap<String, Int> = {}
    
    receive {
        Store(key: String, value: Int) => {
            data[key] = value
        }
        
        Get(key: String, reply: ActorRef<Int>) => {
            reply.send(data[key])
        }
    }
}
```

### C Interoperability

Call C functions directly:

```aether
// Import C function
extern fn sqrt(x: f64) -> f64

actor MathService {
    receive {
        CalculateSqrt(x, reply) => {
            let result = sqrt(x as f64)
            reply.send(result)
        }
    }
}
```## Benchmarking

Run the multi-core benchmark to verify optimizations:

```bash
# Build and run
gcc -O2 -I. -o build/bench_multicore experiments/concurrency/bench_multicore.c -lpthread
./build/bench_multicore

# Expected output (Intel i7-13700K):
# Simple:    1,536 M msg/sec
# Lock-free: 2,764 M msg/sec
# Speedup:   1.80x
```

## Documentation

- [Getting Started Guide](docs/getting-started.md) - Installation and first steps
- [Language Tutorial](docs/tutorial.md) - Learn Aether syntax and concepts
- [Language Reference](docs/language-reference.md) - Complete language specification
- [Architecture Overview](docs/architecture.md) - Runtime and compiler design
- [Type System Guide](docs/gradual-typing.md) - Gradual typing and inference
- [Standard Library](docs/stdlib-reference.md) - Collections, I/O, networking
- [Docker Setup](docker/README.md) - Container development environment

## Development

### Running Tests

```bash
# All tests
make test

# Specific test suite
./build/test_harness compiler    # Compiler tests
./build/test_harness runtime     # Runtime tests
./build/test_harness integration # Integration tests
```

### Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Code of Conduct

This project follows the [Code of Conduct](CODE_OF_CONDUCT.md). By participating, you agree to uphold this code.

## Roadmap

### Current Release (v0.1-alpha)
- Core compiler and runtime
- Actor-based concurrency
- Gradual type system
- C code generation
- Standard library basics

### Next Release (v0.2)
- [ ] Package manager
- [ ] Language Server Protocol (LSP)
- [ ] VS Code extension
- [ ] Improved error messages
- [ ] More stdlib modules

### Future
- [ ] JIT compilation
- [ ] GPU actor support
- [ ] Distributed actors
- [ ] Hot code reloading

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Erlang/OTP for actor model inspiration
- Go for gradual typing concepts
- Rust for modern systems programming practices
- C for providing the compilation target

## Contact

- GitHub: [@nicolasmd87](https://github.com/nicolasmd87)
- Project: [github.com/nicolasmd87/aether](https://github.com/nicolasmd87/aether)

---

**Built with performance and simplicity in mind.**


# Cross-Language Actor Benchmarks

Professional, reproducible comparison of actor message passing performance across programming languages.

## 🎯 Overview

This benchmark suite measures **raw message passing performance** between actors/goroutines/processes in different languages using idiomatic code and fair optimization levels.

**Visualization:** Interactive web dashboard served by **Aether's HTTP server** (dogfooding!)

## 🚀 Quick Start

### Option 1: View Sample Results

```powershell
cd visualize
.\server.exe    # Start Aether HTTP server
# Open http://localhost:8080
```

### Option 2: Run Full Benchmarks

```powershell
# Run all benchmarks
python run_benchmarks.py

# Start visualization server
cd visualize
.\server.exe
# Open http://localhost:8080
```

## 📊 What's Measured

**Test:** Ping-pong 10M messages between two actors
- **Throughput:** Messages per second
- **Latency:** Cycles per message (RDTSC)
- **Overhead:** Runtime/scheduler cost

## 🔧 Building

### Aether Server

```powershell
# Automated build
.\build_server.ps1

# Manual build
cd visualize
gcc -O2 -o server.exe server.c -I../../std/net -I../../runtime \
    ../../std/net/*.c ../../runtime/**/*.c -lws2_32 -lpthread
```

### Individual Benchmarks

**Aether (already built):**
```powershell
cd ../../tests/runtime
gcc -O3 -march=native -o bench_batched_atomic.exe bench_batched_atomic.c -lpthread
```

**C++:**
```powershell
cd cpp
g++ -O3 -std=c++17 -march=native -o ping_pong.exe ping_pong.cpp -lpthread
```

**Rust:**
```powershell
cd rust
cargo build --release
```

**Go:**
```powershell
cd go
go build ping_pong.go
```

## 📁 Structure

```
cross-language/
├── README.md              # This file
├── run_benchmarks.py      # Automated benchmark runner
├── build_server.ps1       # Build Aether HTTP server
│
├── visualize/             # Web dashboard
│   ├── server.c          # Aether HTTP server (serves dashboard)
│   ├── index.html        # Interactive dashboard (Plotly.js)
│   └── results.json      # Benchmark results (generated)
│
├── aether/               # Aether benchmarks (uses existing tests)
├── cpp/                  # C++ benchmarks
├── rust/                 # Rust benchmarks
└── go/                   # Go benchmarks
```

## 🎨 Dashboard Features

✅ **Interactive graphs** - Plotly.js with zoom, pan
✅ **Export to PNG/SVG** - Publication quality for papers
✅ **Responsive design** - Mobile-friendly
✅ **Auto-refresh** - Updates every 30 seconds
✅ **Detailed tables** - All metrics in one view
✅ **Professional styling** - Academic paper ready

## 📏 Methodology

### Fairness Rules

1. **Same hardware** - All benchmarks on same machine
2. **Optimized builds** - `-O3` for compiled, release mode for others
3. **Idiomatic code** - Use each language's best practices
4. **Same algorithm** - Identical ping-pong logic
5. **Warm runs** - Discard first run (JVM warmup, etc.)
6. **Multiple runs** - Average of 5 runs minimum

### What We Measure

- **Message passing latency** (not computation)
- **Scheduler overhead** (context switches, runtime)
- **Memory fence costs** (atomic operations)
- **Cache efficiency** (false sharing, alignment)

### What We Don't Measure

- I/O performance
- Network latency
- Garbage collection (isolated to message passing)
- Business logic complexity

## 🏆 Expected Results

Based on industry benchmarks and our measurements:

| Language | Typical Range | Notes |
|----------|---------------|-------|
| **Aether** | 1.5-2.5 B/sec | Batched atomics, zero-copy |
| **C++** | 2-3 B/sec | Raw threads, no abstraction |
| **Rust** | 0.5-1 B/sec | Tokio async overhead (~15ns) |
| **Go** | 100-300 M/sec | Goroutine scheduler |
| **Erlang** | 10-50 M/sec | Process isolation, copying |

*Your results may vary based on hardware*

## 🔬 Technical Details

### Aether Implementation

Uses existing `bench_batched_atomic.c`:
- Plain `int` counters in hot path
- Batched `atomic_store` every 64 messages
- 10.4x faster than atomic-per-operation
- RDTSC cycle counting for accuracy

### Server Implementation

Pure Aether/C HTTP server:
- Serves static HTML dashboard
- Serves JSON results via REST API
- Demonstrates Aether's web capabilities
- ~200 lines of code

### Visualization

Professional graphs with Plotly.js:
- Bar charts (throughput, latency)
- Detailed results table
- Export to PNG/SVG/PDF
- Mobile responsive

## 📖 Citation

If you use these benchmarks in research or presentations:

```
Aether Cross-Language Actor Benchmarks
https://github.com/yourusername/aether
```

## 🤝 Contributing

Pull requests welcome for:
- New language implementations
- New benchmark patterns
- Performance improvements (must be idiomatic!)
- Bug fixes

### Adding a Language

1. Create folder: `<language>/`
2. Implement ping-pong benchmark
3. Output format: `Throughput: X M msg/sec` and `Cycles/msg: Y`
4. Update `run_benchmarks.py`
5. Test with `python run_benchmarks.py`

## 📜 License

Same as Aether project (check root LICENSE file)

## 🙏 Acknowledgments

- **Computer Language Benchmarks Game** - Methodology inspiration
- **TechEmpower Benchmarks** - Visualization inspiration
- **Plotly.js** - Publication-quality graphs

---

**Built with Aether** 🚀 | **Served by Aether HTTP Server** 🌐

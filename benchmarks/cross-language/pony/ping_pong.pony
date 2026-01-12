"""
Pony Ping-Pong Benchmark
Actor-based message passing with Pony's causal messaging
"""

actor Pong
  let _main: Main
  var _count: U64 = 0
  
  new create(main: Main) =>
    _main = main
  
  be pong() =>
    _count = _count + 1
    if _count < 10_000_000 then
      _main.ping()
    else
      _main.done(_count)
    end

actor Main
  let _env: Env
  let _pong: Pong
  var _start: U64 = 0
  var _count: U64 = 0
  
  new create(env: Env) =>
    _env = env
    _pong = Pong(this)
    _env.out.print("=== Pony Ping-Pong Benchmark ===")
    _env.out.print("Messages: 10000000\n")
    _start = @ponyint_cpu_tick[U64]()
    ping()
  
  be ping() =>
    _count = _count + 1
    _pong.pong()
  
  be done(count: U64) =>
    let finish = @ponyint_cpu_tick[U64]()
    let cycles = finish - _start
    let cycles_per_msg = cycles.f64() / 10_000_000
    let throughput = 3000.0 / cycles_per_msg
    
    _env.out.print("Total cycles: " + cycles.string())
    _env.out.print("Cycles/msg: " + cycles_per_msg.string())
    _env.out.print("Throughput: " + throughput.string() + " M msg/sec")

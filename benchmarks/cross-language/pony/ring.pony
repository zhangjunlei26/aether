"""
Pony Ring Benchmark
100 actors passing token in circle
"""

actor RingActor
  let _id: U64
  let _next: (RingActor | Main)
  var _count: U64 = 0
  
  new create(id: U64, next: (RingActor | Main)) =>
    _id = id
    _next = next
  
  be pass(value: U64) =>
    _count = _count + 1
    match _next
    | let n: RingActor => n.pass(value + 1)
    | let m: Main => m.complete(_count)
    end

actor Main
  let _env: Env
  var _start: U64 = 0
  let _ringSize: U64 = 100
  let _rounds: U64 = 100_000
  
  new create(env: Env) =>
    _env = env
    _env.out.print("=== Pony Ring Benchmark ===")
    _env.out.print("Ring size: 100 actors")
    _env.out.print("Rounds: 100000\n")
    
    // Build ring
    var actors = Array[RingActor]
    var i: U64 = 0
    while i < _ringSize do
      actors.push(RingActor(i, this))
      i = i + 1
    end
    
    // Link ring
    i = 0
    while i < (_ringSize - 1) do
      try actors(i.usize())?.changeNext(actors((i + 1).usize())?) end
      i = i + 1
    end
    
    _start = @ponyint_cpu_tick[U64]()
    try actors(0)?.pass(0) end
  
  be complete(count: U64) =>
    let finish = @ponyint_cpu_tick[U64]()
    let cycles = finish - _start
    let totalMsg = _ringSize * _rounds
    let cyclesPerMsg = cycles.f64() / totalMsg.f64()
    let throughput = 3000.0 / cyclesPerMsg
    
    _env.out.print("Total messages: " + totalMsg.string())
    _env.out.print("Cycles/msg: " + cyclesPerMsg.string())
    _env.out.print("Throughput: " + throughput.string() + " M msg/sec")

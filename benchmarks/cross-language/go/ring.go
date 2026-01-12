package main

import (
	"fmt"
	"sync/atomic"
	"time"
)

const RING_SIZE = 100
const ROUNDS = 100000

type RingActor struct {
	value    int64
	received int64
}

func main() {
	fmt.Println("=== Ring Benchmark (Go) ===")
	fmt.Printf("Ring size: %d actors\n", RING_SIZE)
	fmt.Printf("Rounds: %d\n\n", ROUNDS)

	actors := make([]RingActor, RING_SIZE)

	start := time.Now()

	// Initialize
	atomic.StoreInt64(&actors[0].value, 1)
	atomic.StoreInt64(&actors[0].received, 1)

	currentPos := 0
	for round := 0; round < ROUNDS; round++ {
		for i := 0; i < RING_SIZE; i++ {
			next := (currentPos + 1) % RING_SIZE
			val := atomic.LoadInt64(&actors[currentPos].value)
			atomic.StoreInt64(&actors[next].value, val+1)
			atomic.AddInt64(&actors[next].received, 1)
			currentPos = next
		}
	}

	elapsed := time.Since(start)

	totalMessages := ROUNDS * RING_SIZE
	totalCycles := float64(elapsed.Nanoseconds()) * 3.0
	cyclesPerMsg := totalCycles / float64(totalMessages)
	msgPerSec := float64(totalMessages) / elapsed.Seconds()

	fmt.Printf("Total messages: %d\n", totalMessages)
	fmt.Printf("Cycles/msg: %.2f\n", cyclesPerMsg)
	fmt.Printf("Throughput: %.0f M msg/sec\n", msgPerSec/1000000)
}

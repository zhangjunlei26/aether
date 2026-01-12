package main

import (
	"fmt"
	"sync/atomic"
	"time"
)

const NUM_ACTORS = 1000
const MESSAGES_PER_ACTOR = 1000

type Actor struct {
	counter int64
	id      int
}

func main() {
	fmt.Println("=== Skynet Benchmark (Go) ===")
	fmt.Printf("Actors: %d\n", NUM_ACTORS)
	fmt.Printf("Messages per actor: %d\n\n", MESSAGES_PER_ACTOR)

	actors := make([]Actor, NUM_ACTORS)

	start := time.Now()

	for i := 0; i < NUM_ACTORS; i++ {
		actors[i].id = i
		for m := 0; m < MESSAGES_PER_ACTOR; m++ {
			atomic.AddInt64(&actors[i].counter, 1)
		}
	}

	// Aggregate
	var total int64
	for i := 0; i < NUM_ACTORS; i++ {
		total += atomic.LoadInt64(&actors[i].counter)
	}

	elapsed := time.Since(start)

	totalMessages := NUM_ACTORS * MESSAGES_PER_ACTOR
	totalCycles := float64(elapsed.Nanoseconds()) * 3.0
	cyclesPerMsg := totalCycles / float64(totalMessages)
	msgPerSec := float64(totalMessages) / elapsed.Seconds()

	fmt.Printf("Total messages: %d\n", totalMessages)
	fmt.Printf("Total sum: %d\n", total)
	fmt.Printf("Cycles/msg: %.2f\n", cyclesPerMsg)
	fmt.Printf("Throughput: %.0f M msg/sec\n", msgPerSec/1000000)
}

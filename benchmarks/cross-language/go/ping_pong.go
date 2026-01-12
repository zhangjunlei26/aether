package main

import (
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

const MESSAGES = 10000000

func pingPong() (float64, float64) {
	var pingCounter, pongCounter int64
	var wg sync.WaitGroup
	wg.Add(2)

	start := time.Now()

	// Ping goroutine
	go func() {
		defer wg.Done()
		for i := int64(0); i < MESSAGES; i++ {
			atomic.AddInt64(&pingCounter, 1)
			// Wait for pong
			for atomic.LoadInt64(&pongCounter) < i {
				// Yield
			}
		}
	}()

	// Pong goroutine
	go func() {
		defer wg.Done()
		for i := int64(0); i < MESSAGES; i++ {
			// Wait for ping
			for atomic.LoadInt64(&pingCounter) <= i {
				// Yield
			}
			atomic.AddInt64(&pongCounter, 1)
		}
	}()

	wg.Wait()
	elapsed := time.Since(start)

	// Estimate cycles (assuming 3GHz)
	totalCycles := float64(elapsed.Nanoseconds()) * 3.0
	cyclesPerMsg := totalCycles / MESSAGES
	msgPerSec := MESSAGES / elapsed.Seconds()

	return msgPerSec, cyclesPerMsg
}

func main() {
	fmt.Println("=== Go Ping-Pong Benchmark ===")
	fmt.Printf("Messages: %d\n\n", MESSAGES)

	msgPerSec, cyclesPerMsg := pingPong()

	fmt.Printf("Cycles/msg:     %.2f\n", cyclesPerMsg)
	fmt.Printf("Throughput:     %.0f M msg/sec\n", msgPerSec/1000000)
}

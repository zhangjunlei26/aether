"""
Scala Akka Ping-Pong Benchmark
Classic actor framework comparison
"""

import akka.actor.{Actor, ActorSystem, Props}
import scala.concurrent.duration._
import scala.concurrent.{Await, Promise}

case object Ping
case object Pong
case object Start
case class Done(count: Long)

class PingActor(pong: akka.actor.ActorRef, promise: Promise[Long]) extends Actor {
  var count = 0L
  val maxCount = 10000000L
  var startTime = 0L

  def receive = {
    case Start =>
      startTime = System.nanoTime()
      count = 0
      pong ! Pong
      
    case Ping =>
      count += 1
      if (count < maxCount) {
        pong ! Pong
      } else {
        val endTime = System.nanoTime()
        val elapsed = endTime - startTime
        promise.success(elapsed)
        context.system.terminate()
      }
  }
}

class PongActor(ping: akka.actor.ActorRef) extends Actor {
  def receive = {
    case Pong =>
      ping ! Ping
  }
}

object PingPongBenchmark extends App {
  println("=== Scala Akka Ping-Pong Benchmark ===")
  println("Messages: 10000000\n")
  
  val system = ActorSystem("PingPong")
  val promise = Promise[Long]()
  
  val ping = system.actorOf(Props(classOf[PingActor], null, promise))
  val pong = system.actorOf(Props(classOf[PongActor], ping))
  
  // Update ping with pong reference
  val pingWithPong = system.actorOf(Props(classOf[PingActor], pong, promise))
  
  pingWithPong ! Start
  
  val elapsed = Await.result(promise.future, 60.seconds)
  val cycles = (elapsed.toDouble / 1e9) * 3e9 // Estimate cycles at 3GHz
  val cyclesPerMsg = cycles / 10000000
  val throughput = 3000.0 / cyclesPerMsg
  
  println(s"Cycles/msg: ${cyclesPerMsg}")
  println(s"Throughput: ${throughput.toLong} M msg/sec")
  
  Await.result(system.whenTerminated, 10.seconds)
}

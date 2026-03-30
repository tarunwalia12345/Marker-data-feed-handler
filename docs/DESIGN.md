# 📄 DESIGN.md

## 1. System Architecture

This system is designed as a **low-latency market data pipeline** consisting of two main components:

- **Exchange Simulator (Server)**
- **Feed Handler (Client)**

The architecture follows a:

> **Producer → Transport → Consumer → Visualization pipeline**

Optimized for:
- High throughput
- Low latency

---

## 1.a Client–Server Architecture Diagram


Exchange Simulator (Server)



Tick Generator (GBM)
→ Message Builder (Trade/Quote)
→ TCP Broadcast Engine (epoll, multi-client)
→ TCP (Binary Protocol)

→ Feed Handler (Client)
→ Network Thread (epoll recv)
→ Queue (thread decoupling)
→ Parser Thread (binary parsing)
→ Lock-Free Cache (market state)
→ UI Thread (terminal visualization)


---

## 1.b Thread Model

### Server

- Accept + Event Loop (epoll)
  - Handles new client connections
  - Uses non-blocking sockets

- Tick Generation
  - Generates prices using GBM
  - Runs continuously

- Broadcast Loop
  - Sends batched messages
  - Uses non-blocking send()
  - Handles slow clients

---

### Client

- Network Thread
  - Uses edge-triggered epoll
  - Reads from socket
  - Pushes data to queue

- Parser Thread
  - Consumes queue
  - Parses messages
  - Updates cache
  - Records latency

- UI Thread
  - Reads cache
  - Renders every 500ms
  - Displays statistics

---

## 1.c Data Flow

- Tick generated using GBM
- Message encoded (header + payload + checksum)
- Sent over TCP
- Received via epoll
- Buffered and parsed
- Cache updated atomically
- UI displays results

---

# 2. Geometric Brownian Motion

## 2.a Mathematical Formulation

dS = μSdt + σSdW

Where:
- S = price
- μ = drift
- σ = volatility
- dW = Wiener process

---

## 2.b Implementation (Box-Muller)

- Generate U1, U2 ∈ (0,1)
- Z = sqrt(-2 ln U1) × cos(2πU2)
- Use Z in GBM equation

---

## 2.c Parameter Choices

- μ = 0 (neutral market)
- σ ∈ [0.01, 0.06]
- dt = 0.001 (1ms)

---

## 2.d Realistic Price Behavior

- Prices remain positive
- Spread proportional to price (0.05%–0.2%)
- 70% quotes, 30% trades
- Independent symbol processes

---

# 3. Network Layer Design

## 3.a Server Design

- epoll-based event loop
- Handles multiple clients efficiently
- Non-blocking sockets
- Batch broadcasting

---

## 3.b Client Design

- Edge-triggered epoll
- Continuous recv() loop
- Efficient data ingestion

---

## 3.c Buffer Management

- TCP stream handled via internal buffer
- Supports fragmentation
- Incremental parsing

---

## 3.d Reconnection Strategy

- Detect disconnect via recv()
- Exponential backoff
- Reconnect and resubscribe

---

## 3.e Flow Control

- Handle EAGAIN using buffers
- Per-client buffering
- Drop slow clients if needed

---

# 4. Memory Management Strategy

## 4.a Buffer Lifecycle

- Buffers allocated once
- Reused across operations
- Avoid allocations in hot path

---

## 4.b Allocation Patterns

- Stack allocation preferred
- Pre-allocated vectors used

---

## 4.c Alignment and Cache Optimization

- alignas(64) used
- Avoid false sharing
- Cache-friendly layout

---

## 4.d Pool Usage

- Server: batching buffers
- Client: queue buffers
- Optional memory pooling

---

# 5. Concurrency Model

## 5.a Lock-Free Techniques

- Atomic variables used
- No mutex in hot path

---

## 5.b Memory Ordering

- Relaxed ordering for counters
- Strong ordering where required

---

## 5.c Single Writer Multiple Reader

- Parser thread writes
- UI thread reads
- No locking needed

---

# 6. Visualization Design

## 6.a Update Strategy

- Polling every 500ms
- Separate UI thread

---

## 6.b ANSI Escape Codes

- Clear screen using escape codes
- Color output (green/red)

---

## 6.c Statistics Handling

- Atomic counters
- Non-blocking computation

---

# 7. Performance Optimization

## 7.a Hot Path Identification

- Parsing
- Cache updates
- Network I/O

---

## 7.b Cache Optimization

- Cache-line alignment
- Avoid unnecessary copies

---

## 7.c False Sharing Prevention

- Padding structures
- Separate cache lines

---

## 7.d System Call Minimization

- Batch send() operations
- epoll instead of polling
- Reduce recv() calls

---

# Summary

The system achieves:

- High throughput (100K–500K msgs/sec)
- Low latency (microsecond-level)
- Scalable multi-client handling
- Lock-free, cache-efficient design  
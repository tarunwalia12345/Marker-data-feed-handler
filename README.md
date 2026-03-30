# Market Data Feed Handler

## 📌 Overview

Modern electronic trading systems rely on fast and reliable market data feeds to make real-time decisions. This project implements a **low-latency market data feed handler system** that simulates how trading systems receive, process, and utilize live market data.

The system is composed of two main components:

* An **Exchange Simulator**, which generates realistic market data ticks
* A **Feed Handler**, which receives, parses, and maintains market state efficiently

The design focuses on key principles required for high-performance systems:

* Efficient data generation using **Geometric Brownian Motion (GBM)**
* High-throughput communication using a **binary protocol over TCP**
* Event-driven processing via **epoll-based non-blocking I/O**
* Consistent and fast state access using a **lock-free symbol cache**
* Real-time monitoring through **terminal visualization and latency metrics**

This project demonstrates how to build a scalable and efficient pipeline for handling continuous streams of market data.

---

## 🏗️ System Architecture

The system follows a client-server model:

Exchange Simulator (Server)
→ TCP (Binary Protocol)
→ Feed Handler (Client - epoll)
→ Parser → Cache → Visualization

The server continuously generates market data, while the client processes and displays it in real time.

---

## ⚙️ Components

### 1. Exchange Simulator (Server)

The exchange simulator is responsible for generating and broadcasting market data.

* Generates price movements using **Geometric Brownian Motion**
* Maintains multiple symbols (as required by the assignment)
* Uses TCP sockets for communication
* Broadcasts updates to all connected clients
* Generates both trade and quote messages
* Assigns sequence numbers and timestamps to each message

---

### 2. Feed Handler (Client)

The feed handler connects to the server and processes incoming data efficiently.

* Uses **epoll** for event-driven I/O
* Operates with non-blocking sockets
* Handles TCP stream fragmentation
* Continuously reads and processes incoming data
* Supports reconnection on disconnection

---

### 3. Binary Protocol

A compact binary protocol is used for efficient communication.

Header fields:

* Message Type (Trade / Quote)
* Sequence Number
* Timestamp
* Symbol ID

Payload:

* Trade → Price, Quantity
* Quote → Bid, Ask, Sizes

This design minimizes overhead and enables fast parsing.

---

### 4. Binary Parser

The parser converts raw byte streams into structured messages.

* Uses buffer-based parsing to handle partial reads
* Processes messages only when fully received
* Supports both Trade and Quote message types
* Detects sequence gaps
* Avoids unnecessary memory allocation

---

### 5. Market Data Cache

The cache stores the latest state for each symbol.

* Lock-free design using atomic variables
* Single writer (parser), multiple readers (visualization)
* Provides fast read access
* Maintains consistency without locks

---

### 6. Visualization

A terminal-based interface displays market data.

* Periodically refreshes output
* Shows symbol-wise data such as:

  * Bid / Ask
  * Last traded price
  * Update counts
* Uses ANSI escape codes for rendering
* Designed to avoid blocking the processing loop

---

### 7. Latency Tracking

Latency tracking is used to measure system performance.

* Records processing latency
* Computes statistics such as:

  * p50
  * p99
  * p999
* Uses efficient data structures to avoid overhead

---

## 📊 Performance

### Server

* Continuously generates market data
* Handles multiple clients
* Uses non-blocking operations to avoid delays

### Client

* Efficiently processes incoming messages
* Maintains low-latency updates in cache
* Stable under continuous data flow

### End-to-End Flow

T0 (server timestamp)
→ Network transmission
→ Parsing
→ Cache update
→ Visualization

---

## 🧠 Design Decisions

* **epoll** is used for scalable and efficient I/O handling
* **Binary protocol** reduces message size and parsing overhead
* **Lock-free cache** avoids synchronization bottlenecks
* **Buffer-based parsing** ensures correct handling of TCP streams
* **Non-blocking sockets** prevent delays in data processing

---

## 🧪 Assignment Coverage

This implementation satisfies all key requirements:

✔ Exchange simulator using GBM
✔ TCP-based communication
✔ epoll-based client
✔ Binary message parsing
✔ Lock-free symbol cache
✔ Terminal visualization
✔ Latency measurement
✔ Reconnection handling

---

## 📂 Project Structure

src/
server/ → exchange simulator
client/ → feed handler
common/ → shared components

include/ → header files
docs/ → design and analysis documents
benchmarks/ → latency measurement
scripts/ → build and run scripts

---

## 🚀 Build & Run

Build:
./scripts/build.sh

Run Server:
./scripts/run_server.sh

Run Client:
./scripts/run_client.sh

Run Demo:
./scripts/run_demo.sh

Benchmark:
./scripts/benchmark_latency.sh

---

## 🧩 Key Challenges

* Handling TCP stream fragmentation correctly
* Designing an efficient binary parser
* Maintaining consistency in a lock-free cache
* Avoiding blocking operations in critical paths
* Measuring latency with minimal overhead

---

## 🎯 Conclusion

This project demonstrates:

* Efficient network programming techniques
* Event-driven system design using epoll
* Use of lock-free data structures
* Real-time processing of streaming data

It provides a complete implementation of a **market data feed handler system** aligned with low-latency system design principles.

---

## 👨‍💻 Author

Tarun Walia

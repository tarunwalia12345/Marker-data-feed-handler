# 🚀 Market Data Feed Handler

> A low-latency, high-throughput market data processing system simulating an NSE-style co-location trading environment.

This project was built as part of a systems design assignment to understand how real-world trading infrastructure handles **high-frequency data streams under strict latency constraints**.

---

## ⚡ What This Project Does

At a high level, the system:

* Simulates an exchange generating market ticks
* Streams data over TCP using a binary protocol
* Processes the data in a low-latency feed handler
* Maintains real-time market state
* Displays live updates with performance metrics

---

## 🏗️ Architecture Overview

```id="arch1"
Exchange Simulator (Server)
    ├── Tick Generator (GBM)
    ├── Message Builder (Trade/Quote)
    └── TCP Broadcast (epoll)
            │
            ▼
Feed Handler (Client)
    ├── TCP Socket (epoll)
    ├── Zero-Copy Parser
    ├── Lock-Free Cache
    └── Latency Tracker
            │
            ▼
Terminal Visualization
```

---

## 📦 Project Structure

```id="struct1"
├── src/
│   ├── server/        # Exchange simulator (tick generation + TCP server)
│   ├── client/        # Feed handler (socket + parser + UI)
│   └── common/        # Protocol, cache, utilities
├── include/           # Headers
├── tests/             # Unit tests
├── benchmarks/        # Performance benchmarks
├── docs/              # Design documentation
├── scripts/           # Build & run scripts
├── CMakeLists.txt
└── README.md
```

---

## 🔌 Binary Protocol Design

Each message consists of:

**Header (16 bytes)**

* Type (2B) → Trade / Quote / Heartbeat
* Sequence Number (4B) → Strictly increasing
* Timestamp (8B) → Nanoseconds
* Symbol ID (2B)

**Payload**

* Trade → Price + Quantity
* Quote → Bid/Ask + Quantity

---

## 📊 Performance Results

| Metric             | Observed Value |
| ------------------ | -------------- |
| Throughput         | 100K+ msgs/sec |
| Tick Rate          | Up to 500K/sec |
| Cache Read Latency | < 50 ns        |
| End-to-End Latency | p99 < 50 µs    |

---

## 📈 Performance Graphs

### Throughput Scaling

```id="graph1"
Msgs/sec
500K |                        ██████████████
400K |                   ██████████████████
300K |              ███████████████████
200K |         █████████████████
100K |    ███████████████
  0  |__________________________________
        10K   50K   100K   250K   500K
```

### Latency Distribution

```id="graph2"
Latency (µs)
120 |                      █
100 |                    ███
 80 |                 █████
 60 |              ████████
 40 |         ██████████████
 20 |   █████████████████████
  0 |__________________________________
       p50   p95   p99   p999
```

> The system maintains stable latency even under high throughput, which is critical for trading systems.

---

## ▶️ How to Run

### 🐧 Install Dependencies

```bash id="cmd1"
sudo apt update
sudo apt install -y build-essential cmake g++ make
```

---

### ⚙️ Build (CMake)

```bash id="cmd2"
git clone https://github.com/tarunwalia12345/Marker-data-feed-handler
cd Marker-data-feed-handler

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

### 🚀 Run

#### Start Server

```bash id="cmd3"
./exchange_simulator
```

#### Start Client (new terminal)

```bash id="cmd4"
./feed_handler
```

---

## 📺 Sample Output

```id="output1"
=== NSE Market Data Feed Handler ===

Connected: localhost:9876
Msgs: 1,234,567 | Rate: 98,543 msg/s

Symbol    Bid      Ask      LTP      Volume
-------------------------------------------
RELIANCE  2450.25  2450.75  2450.50  1,234,567
TCS       3678.50  3679.00  3678.75  987,654

Latency:
p50=15µs  p99=45µs  p999=120µs
```

---

## 🧠 Key Design Decisions

* **Single-writer, multi-reader model** → avoids locking overhead
* **Atomic operations instead of mutexes** → predictable latency
* **Edge-triggered epoll** → fewer syscalls, better performance
* **Zero-copy parsing** → eliminates heap allocations
* **Cache-aligned data structures** → better CPU efficiency

---

## ⚙️ Performance Optimizations

* Pre-allocated buffers (no malloc in hot path)
* Minimal syscalls in event loop
* Efficient TCP buffer usage
* False sharing avoidance
* Batched processing

---

## 📚 Documentation

* `docs/DESIGN.md` → Architecture
* `docs/PERFORMANCE.md` → Benchmarks
* `docs/NETWORK.md` → Socket + epoll
* `docs/GBM.md` → Price simulation

---

## 🎯 What I Learned

This project gave me practical experience with:

* Designing low-latency systems
* Handling high-frequency data streams
* Writing lock-free concurrent code
* Optimizing for CPU cache and memory layout
* Trade-offs between throughput and latency

---

## 🔮 Future Improvements

* Multi-threaded parsing pipeline
* Kernel bypass (io_uring / DPDK)
* Persistent logging + replay
* Web dashboard visualization

---

## 👨‍💻 Author

Tarun Walia

Built as part of a **low-latency systems / HFT-style assignment**.

---

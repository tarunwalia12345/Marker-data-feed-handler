# Architecture & Design Decisions

## 1. System Architecture

### Overview
The system is designed as a high-performance client-server market data pipeline.  
The server simulates an exchange and streams market data to multiple clients.  
The client receives, parses, stores, and visualizes this data in real time.

### Architecture Diagram

Exchange Server  
├── Tick Generator (GBM Model)  
├── Client Manager (epoll-based)  
└── Broadcaster  
↓ TCP  
Client  
├── Network Layer (epoll)  
├── Parser  
├── Symbol Cache  
└── Terminal Visualizer

---

## 2. Thread Model

### Server
- Single-threaded event loop
- Responsibilities:
  - Accept new client connections
  - Generate ticks
  - Broadcast messages

### Client
- Single-threaded epoll loop
- Responsibilities:
  - Receive data
  - Parse messages
  - Update cache
  - Render output

---

## 3. Data Flow

Tick Generator → Encoding → TCP Send  
→ recv() → Parser → Cache Update  
→ Visualizer → Terminal Display

---

## 4. Memory Management Strategy

- Preallocated buffers for network I/O
- Avoid dynamic allocation in hot paths
- Cache-aligned structures (64-byte alignment)
- Reuse memory buffers to reduce overhead

---

## 5. Concurrency Model

- Single-writer (parser updates cache)
- Multiple-readers (visualizer reads cache)
- Atomic variables ensure thread safety
- Lock-free design minimizes latency

---

## 6. Visualization Design

- Display updates every 500ms
- Uses ANSI escape codes
- Clears and redraws terminal efficiently
- Statistics computed without blocking

---

## 7. Performance Optimization

- Minimized system calls
- Cache-friendly data layout
- Avoided false sharing
- Reduced branching in critical paths  
# Performance Analysis

## 1. Measurement Methodology

- Used std::chrono for timing
- Benchmarked using large message volumes
- Calculated mean and percentile metrics

---

## 2. Hardware Specification

- CPU: x86_64
- RAM: 16GB
- OS: Ubuntu (WSL)

---

## 3. Server Metrics

| Metric | Value |
|-------|------|
| Tick rate | ~100K/sec |
| Broadcast latency | <100 µs |
| CPU usage | ~20% |

---

## 4. Client Metrics

| Metric | Value |
|-------|------|
| Parser throughput | ~1M msg/sec |
| Cache latency | <1 µs |
| p50 latency | ~15 µs |
| p99 latency | ~50 µs |

---

## 5. Network Metrics

| Metric | Value |
|-------|------|
| Throughput | ~50 Mbps |
| Packet loss | 0 |
| Reconnection time | <1 sec |

---

## 6. Latency Breakdown

- T0: Server send
- T1: Network
- T2: recv()
- T3: parse
- T4: visualization

---

## 7. Optimization Impact

- Reduced memory allocation improved performance
- Non-blocking I/O increased throughput
- Cache alignment reduced latency  
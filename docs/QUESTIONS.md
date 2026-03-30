# Critical Thinking Questions & Answers

---

# 1. How do you efficiently broadcast to multiple clients without blocking?

Efficient broadcasting is achieved using non-blocking sockets and an event-driven architecture.

The server uses epoll and sends data using send() with MSG_DONTWAIT. Instead of waiting for each client, it iterates over all connected clients and attempts to send data. If a client is not ready, the server skips or buffers the data instead of blocking.

This ensures that a slow client does not impact overall system performance.

---

# 2. What happens when a client's TCP send buffer fills up?

When a client's TCP send buffer is full, send() returns EAGAIN or EWOULDBLOCK.

This indicates that the kernel cannot accept more data at the moment.

To handle this:
- The unsent data is stored in a per-client buffer
- The server retries sending this data later

If the buffer grows too large, the client may be considered slow and disconnected.

---

# 3. How do you ensure fair distribution when some clients are slower?

Fairness is ensured by:

- Avoiding blocking on any single client
- Using per-client buffers
- Skipping slow clients temporarily

Optional strategies include:
- Round-robin scheduling
- Limiting per-client buffer size

This ensures that fast clients are not affected by slow ones.

---

# 4. How would you handle 1000+ concurrent client connections?

Handling a large number of clients requires:

- epoll-based event handling (O(1) scalability)
- Non-blocking sockets
- Avoiding one thread per client

Optimizations include:
- Message batching
- Efficient memory usage
- Minimizing system calls

This allows the system to scale efficiently to thousands of clients.

---

# 5. Why use epoll edge-triggered instead of level-triggered for feed handler?

Edge-triggered epoll reduces redundant notifications and improves performance.

In level-triggered mode, the same event may be repeatedly notified until handled, increasing overhead.

In edge-triggered mode:
- Notifications occur only when state changes
- Fewer system calls are required

However, it requires draining the socket completely, making implementation slightly more complex.

---

# 6. How do you handle the case where recv() returns EAGAIN/EWOULDBLOCK?

EAGAIN or EWOULDBLOCK indicates that no more data is available to read.

The correct approach is:
- Break out of the recv loop
- Wait for the next epoll notification

It is important to continuously call recv() in a loop until EAGAIN is encountered.

---

# 7. What happens if the kernel receive buffer fills up?

If the kernel receive buffer is full:

- TCP applies backpressure to the sender
- Incoming data may be delayed

This can increase latency and reduce throughput.

To mitigate:
- Increase SO_RCVBUF
- Process incoming data quickly

---

# 8. How do you detect a silent connection drop (no FIN/RST)?

Silent drops are detected using a heartbeat mechanism.

The client tracks the timestamp of the last received message. If no data is received within a threshold (e.g., 5 seconds), the connection is considered dead.

The client then triggers a reconnection.

---

# 9. Should reconnection logic be in the same thread or separate?

Reconnection logic can be implemented in:

- The same thread (simpler design)
- A separate thread (better scalability)

In this implementation, it is handled in the network thread for simplicity.

---

# 10. How do you buffer incomplete messages across multiple recv() calls efficiently?

Since TCP is a stream protocol, messages may arrive in fragments.

The solution is:
- Maintain an internal buffer
- Append incoming data
- Parse only complete messages

Any remaining partial message is preserved for the next recv() call.

---

# 11. What happens when you detect a sequence gap - drop it or request retransmission?

When a sequence gap is detected:

- It is logged as an error
- The message is skipped

In advanced systems, retransmission may be requested. However, in this system, gaps are tolerated for simplicity.

---

# 12. How would you handle messages arriving out of order (if TCP guarantees order, when might this happen)?

TCP guarantees in-order delivery, but out-of-order processing can occur due to:

- Multi-threaded processing
- Multiple data sources
- Application-level bugs

Handling includes:
- Sequence validation
- Ignoring stale messages

---

# 13. How do you prevent buffer overflow with malicious large message lengths?

To prevent overflow:

- Validate message size before parsing
- Set a maximum allowed message size
- Reject invalid messages

This protects against malformed or malicious inputs.

---

# 14. How do you prevent readers from seeing inconsistent state during updates?

A versioning mechanism (sequence lock pattern) is used.

- Writer increments version before and after update
- Reader checks version consistency

If versions mismatch, the read is retried.

This prevents torn reads.

---

# 15. What memory ordering do you need for atomic operations?

- std::memory_order_relaxed for counters and non-critical updates
- Stronger ordering (e.g., acquire-release) when consistency is required

This balances correctness and performance.

---

# 16. How do you handle cache line bouncing with single writer and visualization reader?

- Use cache-line alignment (alignas(64))
- Separate frequently updated variables

Since there is a single writer, contention is minimal.

---

# 17. Do you need read-copy-update (RCU) pattern here?

No, RCU is not required.

Because:
- There is a single writer
- Readers only read snapshots

Lock-free atomic updates are sufficient.

---

# 18. How do you update display without interfering with network/parsing threads?

- UI runs in a separate thread
- Reads from cache only

No locking is required, ensuring no interference with critical paths.

---

# 19. Should you use ncurses or raw ANSI codes? Why?

Raw ANSI codes are preferred because:

- Lower overhead
- Simpler implementation
- No external dependencies

ncurses is more powerful but unnecessary for this use case.

---

# 20. How do you calculate percentage change when prices update continuously?

- Maintain a baseline price
- Compute change as:

((current_price - base_price) / base_price) × 100

Baseline is initialized on first update.

---

# 21. Sorting is O(n log n) - how can you calculate percentiles faster?

Use histogram-based approximation:

- Bucket latencies
- Compute cumulative counts

This allows O(n) processing and near O(1) percentile queries.

---

# 22. How do you minimize the overhead of timestamping?

- Use std::chrono::high_resolution_clock
- Avoid unnecessary conversions
- Measure only critical sections

This keeps overhead minimal.

---

# 23. What granularity of histogram buckets balances accuracy vs memory?

A logarithmic bucket approach is used.

- Finer granularity for low latency values
- Coarser for higher values

This provides a good tradeoff between accuracy and memory usage.
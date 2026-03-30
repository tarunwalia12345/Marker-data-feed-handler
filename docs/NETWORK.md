# Network Implementation Details

---

# 1. Server-Side Design

The server is responsible for generating market data and distributing it efficiently to multiple connected clients. The design prioritizes scalability, low latency, and non-blocking behavior.

---

## 1.a Multi-client epoll Handling

The server uses an event-driven architecture based on the Linux epoll API.

- A non-blocking TCP listening socket is created and registered with epoll
- epoll_wait() is used to monitor readiness of file descriptors
- When the listening socket becomes ready:
    - accept() is called repeatedly until no more connections are pending
    - Each accepted client socket is set to non-blocking mode
    - The client file descriptor is added to the epoll instance

This approach ensures:
- Efficient handling of large numbers of concurrent clients
- O(1) scalability compared to select/poll
- Minimal overhead per connection

---

## 1.b Broadcast Strategy

Market data messages are generated in batches and sent to all connected clients.

Two approaches were considered:

- Iterating over all clients in a single loop (implemented)
- Using a separate broadcast thread

The chosen design uses a single-threaded broadcast loop because:

- It avoids synchronization overhead between threads
- It improves cache locality
- It simplifies error handling

Messages are accumulated into a buffer and sent using non-blocking send() calls.

---

## 1.c Slow Client Detection and Handling

Slow clients are detected when their TCP send buffer becomes full.

- send() returning EAGAIN indicates that the client cannot accept more data
- In this case:
    - The unsent portion of the message is stored in a per-client buffer
    - The server retries sending this data in future iterations

To prevent system degradation:

- A threshold is defined for per-client buffer size
- If a client exceeds this threshold:
    - It is considered a slow consumer
    - It may be temporarily skipped or disconnected

This ensures that slow clients do not impact overall system throughput.

---

## 1.d Connection State Management

Each client connection is managed through a simple lifecycle:

- CONNECTED
- DISCONNECTED

On connection:
- Client is added to active connection list

On error:
- Socket is closed
- Client is removed from the list

This ensures clean resource management and avoids file descriptor leaks.

---

# 2. Client-Side Design

The client is responsible for receiving, processing, and visualizing market data efficiently.

---

## 2.a Socket Programming Decisions

- TCP is used for reliable, ordered data delivery
- Sockets are configured as non-blocking
- epoll is used for scalable event-driven I/O

This combination ensures:
- Low latency
- High throughput
- Robust communication

---

## 2.b Why epoll over select/poll?

epoll is preferred due to:

- O(1) scalability with number of file descriptors
- Reduced overhead compared to select/poll (which are O(n))
- Efficient event notification

This makes epoll suitable for high-performance systems.

---

## 2.c Edge-triggered vs Level-triggered

Edge-triggered mode is used for better performance.

Advantages:
- Fewer system calls
- Reduced redundant notifications
- Better scalability under load

Tradeoffs:
- Requires draining the socket completely
- Slightly more complex logic

The implementation ensures recv() is called in a loop until EAGAIN.

---

## 2.d Non-blocking I/O Patterns

All sockets are configured with O_NONBLOCK.

- recv() is called repeatedly until no more data is available
- send() is non-blocking

This prevents:
- Thread blocking
- Latency spikes

It enables continuous processing of incoming data.

---

# 3. TCP Stream Handling

TCP is a stream-oriented protocol and does not preserve message boundaries.

---

## 3.a Message Boundary Detection

Each message consists of:
- Fixed-size header
- Variable payload
- Checksum

The parser determines message boundaries by:
- Reading header first
- Calculating expected message size
- Verifying checksum

---

## 3.b Partial Read Buffering Strategy

Incoming data may arrive in fragments.

- Data is appended to an internal buffer
- Parsing is performed only when a complete message is available

If insufficient data is present:
- The parser waits for the next recv() call

This ensures correct reconstruction of messages.

---

## 3.c Buffer Sizing Calculations

- Socket receive buffer (SO_RCVBUF) is set to ~4MB
- This prevents packet drops under high throughput

- Internal buffers are dynamically managed
- Upper bounds are enforced to prevent memory exhaustion

---

# 4. Connection Management

---

## 4.a Connection State Machine

The client follows a state machine:

DISCONNECTED → CONNECTING → CONNECTED → DISCONNECTED

Transitions:
- On successful connect → CONNECTED
- On error → DISCONNECTED

---

## 4.b Retry Logic and Backoff Algorithm

Reconnection is implemented using exponential backoff:

Delay = 100ms × (2^retry_count)

- Retry count is capped
- Prevents aggressive reconnect attempts
- Reduces load on server

---

## 4.c Heartbeat Mechanism

The client monitors activity to detect silent disconnections.

- Timestamp of last received message is tracked
- If no data is received within a threshold (e.g., 5 seconds):
    - Connection is considered dead
    - Reconnection is triggered

---

# 5. Error Handling

---

## 5.a Network Errors

- EPIPE:
    - Indicates broken connection
    - Client is disconnected

- ECONNRESET:
    - Connection reset by peer
    - Requires reconnection

- EAGAIN / EWOULDBLOCK:
    - Temporary condition
    - Operation retried later

---

## 5.b Application-Level Errors

- Invalid message type
- Checksum mismatch
- Sequence gaps

Handling strategy:
- Skip invalid messages
- Log errors
- Continue processing

---

## 5.c Recovery Strategies

- On socket errors:
    - Close connection
    - Attempt reconnection

- On corrupted data:
    - Resynchronize buffer
    - Continue parsing

This ensures system resilience and continuous operation.


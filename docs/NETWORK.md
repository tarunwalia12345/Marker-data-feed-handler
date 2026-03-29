# Network Implementation Details

## 1. Server-Side Design

- Uses epoll for multi-client handling
- Non-blocking sockets
- Broadcasts messages to all connected clients

### Slow Client Handling
- If send() fails, client is disconnected

---

## 2. Client-Side Design

- epoll-based event loop
- Non-blocking recv()
- Batch message processing

### Why epoll?
- O(1) scalability
- Better performance than select/poll

---

## 3. TCP Stream Handling

- Messages structured as header + payload
- Handles partial reads
- Uses buffer accumulation

---

## 4. Connection Management

- Retry logic with delay
- Automatic reconnection
- Backoff strategy

---

## 5. Error Handling

- EAGAIN → retry later
- ECONNRESET → reconnect
- EPIPE → close connection  
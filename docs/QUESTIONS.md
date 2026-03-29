# Critical Thinking Answers

## 1. How do you update display without blocking?

Display updates run on a periodic timer (500ms)  
and are separated from the network loop.

---

## 2. Should you use ncurses or ANSI codes?

ANSI escape codes are preferred because:
- Lightweight
- No external dependencies
- Faster rendering

---

## 3. How do you calculate percentage change?

Change % = (LTP − Mid Price) / Mid Price × 100

---

## 4. How do you compute percentiles efficiently?

Use histogram-based approximation instead of sorting  
to achieve O(n) complexity.

---

## 5. How do you minimize timestamp overhead?

- Use high_resolution_clock
- Avoid system calls
- Optionally use CPU cycle counters

---

## 6. Histogram bucket tradeoff

- More buckets → higher accuracy
- Fewer buckets → lower memory usage and faster computation  
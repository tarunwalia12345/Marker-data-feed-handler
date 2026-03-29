# Geometric Brownian Motion (GBM)

## 1. Mathematical Background

Geometric Brownian Motion is used to simulate realistic stock price movements.

### Stochastic Differential Equation

dS = μSdt + σSdW

### Discretized Form

S(t + dt) = S(t) × exp((μ − 0.5σ²)dt + σ√dt × Z)

---

## 2. Implementation Details

### Normal Distribution (Box-Muller Transform)

Z = sqrt(-2 ln U1) × cos(2π U2)

---

## 3. Parameter Selection

| Parameter | Description | Value |
|----------|------------|------|
| μ        | Drift       | 0.05 |
| σ        | Volatility  | 0.2  |
| dt       | Time step   | 1 ms |

---

## 4. Realism Considerations

- Bid-ask spread proportional to price
- Volume generated randomly or fixed
- Optional correlation between price and volume  
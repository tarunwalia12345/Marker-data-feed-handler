# Geometric Brownian Motion (GBM)

---

# 1. Mathematical Background

## 1.a Stochastic Differential Equation (SDE)

The price evolution is modeled using the stochastic differential equation:

dS = μSdt + σSdW

Where:
- S = current asset price
- μ = drift (expected return)
- σ = volatility
- dt = time increment
- dW = Wiener process (random noise)

---

## 1.b Discretization for Simulation

For numerical simulation, the SDE is discretized as:

S(t + dt) = S(t) × exp[(μ − σ²/2)dt + σ√dt × Z]

Where:
- Z ~ N(0,1) (standard normal random variable)

This formulation ensures:
- Prices remain strictly positive
- Log returns follow a normal distribution

---

## 1.c Why GBM for Stock Prices?

GBM is widely used in finance because:

- It models continuous price evolution
- It incorporates randomness (volatility)
- It ensures prices never become negative
- It captures realistic market behavior

Although simplified, it is sufficient for simulation of market data feeds.

---

# 2. Implementation Details

## 2.a Box-Muller Transform

To generate standard normal random variables (Z), the Box-Muller transform is used.

Given:
- U1, U2 ∈ (0,1)

Z = sqrt(-2 ln U1) × cos(2πU2)

This produces a normally distributed random variable.

---

## 2.b Parameter Selection

- Drift (μ)
    - Set to 0 for neutral market behavior
    - Can be adjusted for bullish or bearish trends

- Volatility (σ)
    - Randomized per symbol
    - Range: 0.01 to 0.06
    - Higher σ → more price fluctuations

- Initial Price
    - Randomized between ₹100 and ₹5000

---

## 2.c Time Step (dt)

- dt = 0.001 (1 millisecond)

Reason:
- Matches high-frequency tick simulation
- Provides smooth price evolution
- Avoids unrealistic large jumps

---

# 3. Realism Considerations

## 3.a Bid-Ask Spread

Spread is proportional to price:

Spread = Price × (0.05% to 0.2%)

This ensures:
- Tight spreads for liquid instruments
- Wider spreads for volatile assets

---

## 3.b Volume Generation

- Trade quantities are generated using fixed or random small values
- Prevents unrealistic spikes
- Maintains stable order flow

---

## 3.c Correlation Between Price and Volume (Optional)

- Basic model assumes independence between price and volume
- Optional enhancement:
    - Higher volatility → higher trading activity
    - Larger price moves → increased volume

This can be incorporated for more realistic simulations if required.

---

# Summary

The GBM-based implementation ensures:

- Realistic stochastic price movement
- Stable and continuous evolution
- Positive price guarantees
- Efficient computation for high-frequency systems  
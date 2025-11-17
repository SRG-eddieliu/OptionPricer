# OptionPricer — Release Notes

## v1.0.0 — Tester

Date: 2025-11-17

This release contains the initial skeletons and working implementations for multiple option pricing engines and a tester program located at `src/main.cpp`. The tester (`./output/main`) demonstrates three scenarios:

1. European option pricing comparison (Black–Scholes analytic, Binomial CRR, Trinomial tree, Monte Carlo)
2. American option pricing (Binomial / Trinomial), with European vs American comparison and early-exercise premium
3. Convergence comparison showing how binomial and trinomial approaches approach the Black–Scholes reference price

The `src/main.cpp` in this commit is designated the v1.0.0 tester. Verify its output locally; once you confirm, we will create and push the `v1.0.0` tag.

## v1.0.0 Contents

- `src/main.cpp` — v1 tester demonstrating all engines and comparisons
- `src/engines/*` — engine implementations: Black-Scholes analytic, Binomial CRR, Trinomial tree, Monte Carlo (European)
- `src/core/Types.hpp` — option/market types
- `src/math/*` — normal distribution and statistics helpers
- `README.md` — feature list, usage, findings, and roadmap


## v2.0.0 Roadmap (brief)

- Add Least-Squares Monte Carlo (LSMC) to price American options via regression on pathwise continuation values (works within the existing MC framework).
- Add additional Monte Carlo variance-reduction techniques and frameworks:
    - Control variates (e.g., use analytic European price as control)
    - Antithetic variates
    - Importance sampling / stratified sampling
    - Quasi-Monte Carlo (Sobol, Halton) for lower-discrepancy sampling
    - Multi-level Monte Carlo (MLMC) experimentation
- Exotic options pricing via MC: Barrier, Asian, lookback 
- Calibration: Implied vol extraction from market prices


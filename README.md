# OptionPricer

High-performance C++ library for pricing European and American vanilla options with analytical, lattice, and Monte Carlo engines.

## Feature Matrix

| Engine | European | American | Greeks |
|--------|:--------:|:--------:|:------:|
| Black-Scholes Analytic | ✓ | ✗ | ✓ |
| Binomial CRR | ✓ | ✓ | ✓ |
| Trinomial Tree | ✓ | ✓ | ✓ |
| Monte Carlo (European) | ✓ | ✗ | Std dev/err |
| Monte Carlo LSMC (American) | ✗ | ✓ | Std dev/err |

`MCAmericanLSMCEngine` implements the Longstaff–Schwartz regression routine for American exercise (Laguerre basis, configurable degree).

## Architecture Snapshot

- `engines::PricingEngine` defines the polymorphic `price(spec, params)` interface used by every engine, so the demo binary can swap pricing implementations at runtime. Engines inherit from this base, returning the same `PriceOutputs` struct for downstream consumers.
- Inheritance keeps model-specific details encapsulated: `BSEuropeanAnalytic`, `BinomialCRR`, `TrinomialTree`, `MCEuropean`, and `MCAmericanLSMC` all override `price`, while `MCEngine` captures the shared Monte Carlo plumbing (path loops, RNG seeding, variance-reduction hooks) now reused by both European and American MC engines.
- Core data structures live in `src/core/Types.hpp`; math helpers (normal distribution, statistics) live under `src/math/` to keep pricing engines focused on algorithms.
- Boost (via `boost::math::normal`) provides numerically-stable CDF/PDF evaluations for Black–Scholes Greeks. Keeping Boost headers as the sole external dependency preserves portability while leaning on a proven implementation instead of hand-rolled approximations.
- The stack targets C++20 to guarantee deterministic performance, stack allocation, and cache-friendly loops—key for Monte Carlo workloads that launch hundreds of thousands of paths and need predictable latency, mirroring the approach taken in production quant libraries.
- Example/demo code lives under `example/` (see `example_v1.cpp`) so the library can be reused or unit-tested independently of the demo binary.

```
OptionPricer/
├── src/
│   ├── core/Types.hpp
│   ├── engines/
│   │   ├── PricingEngine.hpp
│   │   ├── BSEuropeanAnalytic.{hpp,cpp}
│   │   ├── BinomialCRR.{hpp,cpp}
│   │   ├── TrinomialTree.{hpp,cpp}
│   │   ├── MCEngine.hpp
│   │   ├── MCEuropean.{hpp,cpp}
│   │   └── MCAmericanLSMC.{hpp,cpp}
│   └── math/{Normal,Stats}.{hpp,cpp}
├── example/example_v1.cpp
└── output/
```

## Pricing Methodology

### Analytical Black–Scholes

- Closed-form calls/puts computed via `S e^{-qT} N(d1) - K e^{-rT} N(d2)` (and the corresponding put symmetry), with `d1 = [\ln(S/K) + (r - q + 0.5 σ^2)T]/(σ\sqrt{T})` and `d2 = d1 - σ\sqrt{T}`.
- Greeks (Δ, Γ, Θ, ϑ, ρ) come from differentiating the analytic expression; Boost's `N(x)`/`n(x)` keep these derivative terms numerically stable.
- Degenerate scenarios (`T <= 0`, `σ <= 0`) collapse immediately to intrinsic value so the engine never divides by zero or returns NaNs.
- Example: [`example/black_scholes_example.md`](example/black_scholes_example.md)

### Binomial Tree (CRR)

- Uses a recombining tree with `Δt = T/steps_`, up factor `u = e^{σ\sqrt{Δt}}`, down factor `d = 1/u`, and risk-neutral probability `p = (e^{(r-q)Δt} - d)/(u - d)`.
- Terminal payoffs equal intrinsic value; the backward recursion applies `V = e^{-rΔt}[p V_u + (1 - p)V_d]` for Europeans or `max(intrinsic, continuation)` for American exercise.
- Greeks rely on multiplicative log bumps `S e^{±h}` so the finite-difference spacing is symmetric in percentage terms, which behaves better for deep ITM/OTM contracts.
- Converges linearly to the Black–Scholes value as `steps_ → ∞`, so thousands of steps may be required for high accuracy.
- Example: [`example/binomial_trinomial_example.md`](example/binomial_trinomial_example.md)

### Trinomial Tree

- Extends the lattice to three outcomes: up `u = e^{σ\sqrt{3Δt}}`, down `d = 1/u`, and a middle node. Probabilities `p_u`, `p_m`, `p_d` incorporate the drift term `a = r - q - σ^2/2` so that the first two moments of the process match the continuous-time model.
- Same backward induction and log-bump Greeks as the binomial tree, but the extra branch yields `O(Δt^2)` convergence and smoother Greeks near the strike.
- Benefits over the binomial tree: fewer steps for the same accuracy, reduced oscillation, and better stability for American exercise near early-exercise boundaries.
- Example: [`example/binomial_trinomial_example.md`](example/binomial_trinomial_example.md)

### European Monte Carlo

- Simulates the risk-neutral SDE `dS = (r - q)S dt + σ S dW` using the exact log-normal step `S_T = S_0 \exp((r - q - 0.5σ^2)T + σ\sqrt{T} Z)` with `Z ~ N(0,1)` drawn from `std::mt19937_64`.
- Each path contributes one discounted payoff `e^{-rT} payoff(S_T)`; `math::stats` aggregates the sample mean (price), standard deviation, and standard error `σ̂/√M`.
- `MCEngine::applyVarianceReduction` keeps hooks ready for control variates, antithetic variates, Sobol sequences, etc., as outlined in the roadmap.
- Example: [`example/mc_european_example.md`](example/mc_european_example.md)

### American Monte Carlo (Longstaff–Schwartz LSMC)

- Paths live on a discrete grid of `time_steps_` points, each step advancing `S_t` with the same log-normal increment as above.
- During backward induction, only in-the-money paths contribute to the regression of discounted future cash flows, mirroring the textbook Longstaff–Schwartz routine.
- `laguerreBasis` evaluates an orthogonal basis at each ITM spot and solves the normal equations `X^T X β = X^T Y` using Gaussian elimination with partial pivoting; if the system is singular, the continuation defaults to the sample mean so that the algorithm doesn't explode.
- After regressing, cash flows are discounted one step, compared to intrinsic value, and updated if immediate exercise dominates. A final discount to `t=0` plus an intrinsic check ensures early exercise can occur at inception.
- Example: [`example/mc_american_lsmc_example.md`](example/mc_american_lsmc_example.md)

## Core Types (excerpt)

```cpp
struct PlainVanillaPayoff {
    double strike;
    core::OptionType type;
    double operator()(double spot) const;  // max(±(spot - strike), 0)
};

struct OptionSpec {
    PlainVanillaPayoff payoff;
    core::ExerciseStyle exercise;  // European or American
};

struct OptionParams {
    double S, K, r, q, sig, T;  // Spot, strike, rates, vol, maturity
};

struct PriceOutputs {
    double value, delta, gamma, vega, theta, rho;
    double std_dev, std_error;  // populated by MC engines
};
```

## Pricing Engines

### Black-Scholes (BSEuropeanAnalytic)
Closed-form call/put values `S e^{-qT} N(d1) - K e^{-rT} N(d2)` with analytical Greeks.

### Binomial CRR
Recombining up/down tree with `u = e^{σ√Δt}`, `d = 1/u`, risk-neutral probability `p = (e^{(r-q)Δt} - d) / (u - d)`. Backward induction provides both European and American prices; Greeks computed via multiplicative log bumps.

### Trinomial Tree
Adds a middle node per step for faster convergence. Probabilities tuned via drift term `a = r - q - σ²/2` to match process moments; same early-exercise logic and bump-based Greeks as the binomial engine.

### Monte Carlo (European)
Simulates `S_{i+1} = S_i exp((r - q - σ²/2)Δt + σ√Δt Z_i)` under the risk-neutral measure, discounts mean payoff, and reports sample variance/std error. Hooks exist for variance reduction strategies (control/antithetic/etc.).

### Monte Carlo LSMC (American)
`MCAmericanLSMCEngine` runs the Longstaff–Schwartz regression loop (Laguerre polynomial basis, configurable degree) with backward induction, early-exercise decisions, and MC statistics.

## Build & Run

Prerequisites: C++20 compiler (clang++/g++), Boost headers for normal CDF/PDF implementations.

```bash
brew install boost
mkdir -p output
c++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') -o output/main
```

Demo binary:

```bash
c++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') example/example_v1.cpp -o output/example_v1
./output/example_v1
```

VS Code task runner: `Cmd+Shift+B` (default task builds `output/main`).

## Usage Example

```cpp
core::OptionParams params{100.0, 100.0, 0.05, 0.02, 0.20, 1.0};
core::OptionSpec call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};

engines::BSEuropeanAnalytic bs;
auto bs_px = bs.price(call, params);

engines::BinomialCRREngine tree(4000);
auto tree_px = tree.price(call, params);

core::OptionSpec amer_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::American};
auto amer_px = tree.price(amer_put, params);
```

## Sample Output

Running `./output/example_v1` prints tables comparing analytic, tree, and Monte Carlo prices for European calls/puts, American puts, and convergence diagnostics (binomial vs. trinomial vs. Black–Scholes reference). Monte Carlo output includes standard deviation/error estimates.

## Release & Roadmap

- `RELEASE_NOTES.md` documents v1.0.0 tester details, the v2.0.0 Monte Carlo refactor plan, and now stores a verbatim copy of the historical README for reference.
- Short-term: extend Monte Carlo variance reduction (control variates, antithetic, quasi-MC) and generalize the MC stack for exotic payoffs and calibration workflows.

## Historical README

The previous long-form README has been archived under "Appendix — Historical README Snapshot (pre-cleanup)" in `RELEASE_NOTES.md`. This keeps the same information available while keeping the project root documentation concise.

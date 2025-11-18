# OptionPricer

High-performance C++ library for pricing European and American vanilla options with analytical, lattice, and Monte Carlo engines.

## Feature Matrix

| Engine | European | American | Greeks |
|--------|:--------:|:--------:|:------:|
| Black-Scholes Analytic | ✓ | ✗ | ✓ |
| Binomial CRR | ✓ | ✓ | ✓ |
| Trinomial Tree | ✓ | ✓ | ✓ |
| Monte Carlo (European) | ✓ | ✗ | Δ/Γ via bumps |
| Monte Carlo LSMC (American) | ✗ | ✓ | Std dev/err |

`MCAmericanLSMCEngine` implements the Longstaff–Schwartz regression routine for American exercise (Laguerre basis, configurable degree).

## Architecture Snapshot

- `engines::PricingEngine` defines the polymorphic `price(spec, params)` interface used by every engine.
- `MCEngine` (v2.0.0) captures common Monte Carlo plumbing (path loops, RNG seeding, variance reduction strategy selection).
- Core data structures live in `src/core/Types.hpp`; math helpers (normal distribution, statistics) live under `src/math/`.
- Example/demo code moved to `example/example.cpp` so the library can be reused or unit-tested independently of the demo binary.

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
├── example/example.cpp
└── output/
```

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
c++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') example/example.cpp -o output/example
./output/example
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

Running `./output/example` prints tables comparing analytic, tree, and Monte Carlo prices for European calls/puts, American puts, and convergence diagnostics (binomial vs. trinomial vs. Black–Scholes reference). Monte Carlo output includes standard deviation/error estimates.

## Release & Roadmap

- `RELEASE_NOTES.md` documents v1.0.0 tester details, the v2.0.0 Monte Carlo refactor plan, and now stores a verbatim copy of the historical README for reference.
- Short-term: extend Monte Carlo variance reduction (control variates, antithetic, quasi-MC) and generalize the MC stack for exotic payoffs and calibration workflows.

## Historical README

The previous long-form README has been archived under "Appendix — Historical README Snapshot (pre-cleanup)" in `RELEASE_NOTES.md`. This keeps the same information available while keeping the project root documentation concise.

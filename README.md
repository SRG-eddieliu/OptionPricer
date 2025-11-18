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

- Closed-form calls/puts computed via `S e^{-qT} N(d1) - K e^{-rT} N(d2)` (and the corresponding put symmetry) with Greeks from Boost's normal CDF/PDF helpers.
- Degenerate scenarios (`T <= 0`, `σ <= 0`) collapse immediately to intrinsic value so the engine never divides by zero or returns NaNs.
- Example: [`example/black_scholes_example.md`](example/black_scholes_example.md)

### Binomial & Trinomial Trees

- Recombining lattices approximate the stochastic process over `steps_` discrete dates. Each node compares discounted continuation vs. intrinsic payoff for American options, guaranteeing early-exercise logic identical to textbook CRR/trinomial derivations.
- Greeks use multiplicative log bumps (`spot * exp(± bump_size_)`) to keep finite-difference spacing symmetric in percentage terms, which behaves better for deep ITM/OTM contracts than additive bumps.
- The trinomial engine adds a middle node and drift-adjusted probabilities (`pu`, `pm`, `pd`) to accelerate convergence and reduce oscillations relative to the binomial tree.
- Example: [`example/binomial_trinomial_example.md`](example/binomial_trinomial_example.md)

### European Monte Carlo

- Simulates terminal spots `S_T = S_0 * exp((r - q - σ²/2)T + σ√T Z)` with `Z ~ N(0,1)` using `std::mt19937_64` so simulations are reproducible from the configured seed.
- Each path contributes one discounted payoff (`e^{-rT} * payoff`), and `math::stats` aggregates mean, standard deviation, and standard error.
- `MCEngine::applyVarianceReduction` is the extensibility point for future control variates, antithetic variates, Sobol sequences, etc., mirroring the roadmap in `RELEASE_NOTES.md` without coupling that logic to any specific engine.
- Example: [`example/mc_european_example.md`](example/mc_european_example.md)

### American Monte Carlo (Longstaff–Schwartz LSMC)

- Paths are generated on a discrete grid of `time_steps_`, with each step advancing `S_t` by `exp((r - q - σ²/2)Δt + σ√Δt Z)`. The outer Monte Carlo loop mirrors `MCEngine`, so seeding and future variance reduction strategies stay centralized.
- During backward induction, only in-the-money paths participate in the regression of discounted future cash flows; this matches the classic Longstaff–Schwartz procedure and avoids noisy data from deeply out-of-the-money states.
- `laguerreBasis` produces an orthogonal polynomial basis that behaves well for positive spots; the basis is easy to swap for Chebyshev/Legendre polynomials or even spline bases if a payoff requires it, but Laguerre works well for vanilla payoffs with exponential-like decay.
- `solveNormalEquations` runs Gaussian elimination with partial pivoting on the normal equations. If the matrix becomes singular/near-singular (for example, too few ITM samples or highly colinear basis vectors), the solver returns `false` and the caller falls back to setting continuation equal to the sample mean—keeping the algorithm numerically stable instead of returning wildly oscillating continuation values.
- The routine discounts cash flows after each regression step, enforces immediate exercise at `t=0` when intrinsic value dominates, and finally reports the full MC statistics (value, std dev, std error) alongside the binomial reference numbers shown in `src/main.cpp`.
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

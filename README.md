# OptionPricer

High-performance C++ library for pricing European and American vanilla options with analytical, lattice, and Monte Carlo engines.

## Feature Matrix

| Engine                      | Engine Class            | Feature Highlights                         |
|-----------------------------|-------------------------|-------------------------------------------|
| Black-Scholes Analytic      | `BSEuropeanAnalytic`    | European, Greeks                          |
| Binomial CRR                | `BinomialCRREngine`     | European, American, Greeks                |
| Trinomial Tree              | `TrinomialTreeEngine`   | European, American, Greeks                |
| MC (European Vanilla)       | `MCEuropeanEngine`      | European, variance reduction              |
| MC (American LSMC)          | `MCAmericanLSMCEngine`  | American, variance reduction              |
| MC (Exotic)                 | `MCPathDependentEngine` | Asian, Barrier, Lookback, variance reduction |

*Variance reduction support includes antithetic variates, moment matching, and the combined antithetic+moment strategy via `BaseMCEngine::VarianceReductionMethod`.*

## Architecture Snapshot

*Architecture & Polymorphism*
- The code uses an abstract base class `PricingEngine` which declares a virtual method `price(const core::OptionSpec&, const core::OptionParams&)`.
- Inheritance keeps model-specific details encapsulated: `BSEuropeanAnalytic`, `BinomialCRR`, `TrinomialTree`, `MCEuropean`, `MCAmericanLSMC`, and `MCPathDependentEngine` each handle their pricing logic, while `BaseMCEngine` captures shared Monte Carlo plumbing (path loops, RNG seeding, variance reduction hooks).

*Why C++?*
- **Performance-Critical Workload:** Derivative pricing, especially Monte Carlo simulation, is computationally intensive. C++ offers near-native execution speed without runtime overhead.
- **Memory Efficiency:** Explicit memory control and stack allocation eliminate garbage collection pauses in MC workloads.
- **Industry Standard:** C++ is widely used across banks and hedge funds for production pricing engines.

*Mathematical Libraries*
- Uses **Boost** (`boost::math::distributions::normal`) for accurate normal CDF/PDF computations.

## Project Structure

```
OptionPricer/
├── src/
│   ├── core/Types.hpp
│   ├── engines/
│   │   ├── PricingEngine.hpp
│   │   ├── BSEuropeanAnalytic.{hpp,cpp}
│   │   ├── BinomialCRR.{hpp,cpp}
│   │   ├── TrinomialTree.{hpp,cpp}
│   │   ├── MCEngine.{hpp,cpp}
│   │   ├── MCEuropean.{hpp,cpp}
│   │   ├── MCAmericanLSMC.{hpp,cpp}
│   │   └── MCPathDependent.{hpp,cpp}
│   └── math/{Normal,Stats}.{hpp,cpp}
├── example/
│   ├── example_v1.cpp
│   ├── mc_variance_strategies_example.{cpp,md}
│   └── mc_path_exotics_example.{cpp,md}
├── output/
├── README.md
└── LICENSE
```

## Core Data Structures

### OptionSpec
Defines the option contract specification:
```cpp
struct OptionSpec {
    PlainVanillaPayoff payoff;  // Strike & type (Call/Put)
    ExerciseStyle exercise;     // European or American
};

struct PlainVanillaPayoff {
    double strike;     // Strike price
    OptionType type;   // Call or Put
    double operator()(double ST) const;  // Payoff at spot price ST
};
```

### OptionParams
Defines market parameters:
```cpp
struct OptionParams {
    double S;    // Spot price
    double K;    // Strike price
    double r;    // Risk-free rate
    double q;    // Dividend yield
    double sig;  // Volatility (annualized)
    double T;    // Time to maturity (years)
};
```

### Path-Dependent Specs
```cpp
struct PathDependentOptionSpec {
    ExoticType type;           // ArithmeticAsian, Barrier, Lookback
    OptionType option_type;    // Call or Put
    double strike;
    double barrier_level;
    BarrierType barrier_type;  // Up/Down × In/Out
};
```

## Pricing Methodology

### <span style="text-decoration:underline;">Analytical Black–Scholes</span>
Closed-form formula with Greeks from Boost.

### <span style="text-decoration:underline;">Binomial Tree (CRR)</span>
Recombining tree with log-bump Greeks, handles European & American.

### <span style="text-decoration:underline;">Trinomial Tree</span>
Three-branch tree for faster convergence; same bump logic as binomial.

### <span style="text-decoration:underline;">European Monte Carlo</span>
Terminal-spot simulation: `S_T = S_0 exp((r - q - σ²/2)T + σ√T Z)` under risk-neutral measure.

### <span style="text-decoration:underline;">American Monte Carlo (Longstaff–Schwartz LSMC)</span>
Path grid with backward regression (Laguerre basis) and Gaussian elimination to solve `(X^T X)β = X^T Y`; fallback to sample mean `
 میان تtرا牅ݕ` when singular.

### <span style="text-decoration:underline;">Variance Reduction</span>
Antithetic variates, moment matching, and the combined mode.

### <span style="text-decoration:underline;">Path-Dependent Monte Carlo (Asian/Barrier/Lookback)</span>
`MCPathDependentEngine` computes arithmetic Asian (averaged path), barrier (breach detection), and lookback (running max/min) payoffs. Example: `example/mc_path_exotics_example.md`.

## Build & Run

Install Boost headers, then:
```bash
c++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') -o output/main
```

## License

Released under the [MIT License](LICENSE).

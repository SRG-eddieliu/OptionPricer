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

*Variance Reduction: antithetic variates, moment matching via `BaseMCEngine::VarianceReductionMethod`.*

## Architecture Snapshot

*Architecture & Polymorphism*
- The code uses an abstract base class `PricingEngine` which declares a virtual method `price(const core::OptionSpec&, const core::OptionParams&)`.
- Inheritance keeps model-specific details encapsulated: `BSEuropeanAnalytic`, `BinomialCRR`, `TrinomialTree`, `MCEuropean`, and `MCAmericanLSMC` all override `price`, while `MCEngine` captures the shared Monte Carlo plumbing (path loops, RNG seeding, variance-reduction hooks) now reused by both European and American MC engines.

*Why C++*
- **Performance-Critical Workload:** Derivative pricing, especially Monte Carlo simulation, is computationally intensive. C++ offers near-native execution speed without runtime overhead, unlike Python which requires garbage collection and has inherent interpreter latency.
- **Memory Efficiency:** C++ provides explicit memory control and stack allocation, eliminating the garbage collection pauses that Python imposes. For MC simulations generating hundreds of thousands of price paths, this translates to significantly faster execution with predictable latency.
- **Industry Standard:** C++ remains the dominant language for production pricing stacks across banks and hedge funds

*Mathematical Libraries*
- Leverages **Boost C++ Libraries** (`boost::math::distributions::normal`) for robust, well-tested implementations of the standard normal cumulative distribution function (CDF) and probability density function (PDF), avoiding hand-rolled approximations and ensuring numerical accuracy.


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
│   ├── math/{Normal,Stats}.{hpp,cpp}
│   └── main.cpp
├── example/
│   ├── example_v1.cpp
│   ├── black_scholes_example.{cpp,md}
│   ├── binomial_example.{cpp,md}
│   ├── trinomial_example.{cpp,md}
│   ├── mc_european_example.{cpp,md}
│   ├── mc_american_lsmc_example.{cpp,md}
│   ├── mc_variance_strategies_example.{cpp,md}
│   └── mc_path_exotics_example.{cpp,md}
├── reference/LSMC\ replication.xlsx
├── output/
├── scripts/
└── LICENSE
```

## Core Data Structures

### OptionSpec
Defines the option contract specification:
```cpp
struct OptionSpec {
    PlainVanillaPayoff payoff;           // Strike & type (Call/Put)
    ExerciseStyle exercise;               // European or American
};

struct PlainVanillaPayoff {
    double strike;                        // Strike price
    OptionType type;                      // Call or Put
    
    double operator()(double ST) const;   // Payoff at spot price ST
};
```

### OptionParams
Defines market parameters:
```cpp
struct OptionParams {
    double S;      // Spot price (current stock price)
    double K;      // Strike price
    double r;      // Risk-free rate (annual, continuously compounded)
    double q;      // Dividend yield (annual)
    double sig;    // Volatility (annualized, standard deviation)
    double T;      // Time to maturity (in years)
};
```

### PriceOutputs
Returned by all pricing engines:
```cpp
struct PriceOutputs {
    double value;         // Option price
    double delta;         // ∂V/∂S (price sensitivity to spot)
    double gamma;         // ∂²V/∂S² (delta's sensitivity to spot)
    double vega;          // ∂V/∂σ (price sensitivity to volatility)
    double theta;         // ∂V/∂T (price decay, time value)
    double rho;           // ∂V/∂r (price sensitivity to interest rate)
    double std_dev;       // Standard deviation (MC only)
    double std_error;     // Standard error of estimate (MC only)
};
```

## Pricing Methodology

### <span style="text-decoration:underline;">Analytical Black–Scholes</span>

**Method:** Closed-form solution using the famous Black-Scholes formula:

$$C = S_0 e^{-qT} N(d_1) - K e^{-rT} N(d_2)$$

where:
- $d_1 = \frac{\ln(S_0/K) + (r - q + \sigma^2/2)T}{\sigma\sqrt{T}}$
- $d_2 = d_1 - \sigma\sqrt{T}$
- $N(\cdot)$ is the cumulative standard normal distribution

**Greeks:** Computed analytically using closed-form derivatives.

**Example:** [`example/black_scholes_example.md`](example/black_scholes_example.md)


### <span style="text-decoration:underline;">Binomial Tree (CRR)</span>

**Method:** Constructs a discrete recombining binomial tree over $[0, T]$ with $n$ steps.

At each node:
- Up move: $u = e^{\sigma\sqrt{\Delta t}}$
- Down move: $d = 1/u$
- Risk-neutral probability: $p = \frac{e^{(r-q)\Delta t} - d}{u - d}$

At maturity ($t = T$), payoff = intrinsic value at each leaf node.

Backwards induction from maturity to present:
- European: $V = e^{-r\Delta t} [p \cdot V_u + (1-p) \cdot V_d]$
- American: $V = \max(\text{intrinsic}, e^{-r\Delta t}[\cdots])$ (early exercise check)

**Greeks:** Computed via finite differences:
- Delta: $\frac{V(S + h) - V(S - h)}{2h}$
- Gamma: $\frac{V(S+h) - 2V(S) + V(S-h)}{h^2}$

**Example:** [`example/binomial_example.md`](example/binomial_example.md)


### <span style="text-decoration:underline;">Trinomial Tree</span>

**Method:** Extends binomial to three outcomes per step: up, middle, down.

At each node with step size $\Delta t = T/n$:
- Up move: $u = e^{\sigma\sqrt{3\Delta t}}$
- Down move: $d = 1/u$
- Middle: no move
- Probabilities (tuned for drift): 
  - $p_u = \frac{1}{6} + \frac{a\sqrt{\Delta t}}{2\sigma\sqrt{3}}$
  - $p_d = \frac{1}{6} - \frac{a\sqrt{\Delta t}}{2\sigma\sqrt{3}}$
  - $p_m = \frac{2}{3}$
  - where $a = r - q - \frac{\sigma^2}{2}$

**Convergence:** Faster and more stable convergence compares to Binomial Tree

**Example:** [`example/trinomial_example.md`](example/trinomial_example.md)


### <span style="text-decoration:underline;">European Monte Carlo</span>

**Method:** Stochastic simulation under the risk-neutral measure:

$$dS = (r - q)S\,dt + \sigma S\,dW$$

Discretization (Euler):
$$S_{i+1} = S_i e^{(r-q-\sigma^2/2)\Delta t + \sigma\sqrt{\Delta t} Z_i}$$

where $Z_i \sim N(0,1)$.

1. Simulate $M$ paths of stock prices from $t=0$ to $t=T$
2. Compute payoff at maturity for each path
3. Discount back: $V = e^{-rT} \times \text{mean(payoffs)}$
4. Estimate standard error from sample variance

**Example:**  [`example/mc_european_example.md`](example/mc_european_example.md)


### <span style="text-decoration:underline;">American Monte Carlo (Longstaff–Schwartz LSMC)</span>

**Method:** Full log-normal paths over discrete exercise dates:

$$S_{t+\Delta t} = S_t \exp\bigl((r-q-\tfrac{1}{2}\sigma^2)\Delta t + \sigma\sqrt{\Delta t}\,Z\bigr), \qquad Z \sim N(0,1).$$

Working backward from maturity:

1. Identify the in-the-money paths at the current exercise date.
2. Discount their future cash flows one step ($e^{-r \Delta t}$) and regress those discounted payoffs against Laguerre basis functions of the current spot to estimate continuation.
3. Overwrite the cash flow with intrinsic value whenever `intrinsic > continuation`.
4. Repeat until `t = 0`, then discount once more if needed.

**Laguerre basis:** Evaluate Laguerre polynomials on normalized spots so the regression remains numerically stable even for large $S$.

**Normal equations solver:** Solve $(X^\top X)\beta = X^\top Y$ via Gaussian elimination with partial pivoting; fall back to the sample mean $\bar{Y}$ when the system is singular/ill-conditioned.

**Example:** [`example/mc_american_lsmc_example.md`](example/mc_american_lsmc_example.md)


### <span style="text-decoration:underline;">Path-Dependent Monte Carlo (Asian/Barrier/Lookback)</span>

**Method:** `MCPathDependentEngine` reuses the same path generator with per-path payoff evaluators:
- **Arithmetic Asian:** arithmetic average $\bar{S}$ compared against $K$.
- **Barrier:** tracks barrier hits (Up/Down × In/Out) before applying the terminal payoff.
- **Lookback:** computes payoffs from the running maximum/minimum across the path.

**Example:** [`example/mc_path_exotics_example.md`](example/mc_path_exotics_example.md)


### <span style="text-decoration:underline;">Variance Reduction</span>

#### Antithetic Variates
- Pairs each random draw `Z` with its negation `-Z`, averages the paired payoffs (enabled via `VarianceReductionMethod::AntitheticVariates`), and reduces variance for symmetric payoffs.

#### Moment Matching
- Centers/rescales the simulated normal draws so their sample mean and variance match the theoretical `N(0,1)` moments (select `VarianceReductionMethod::MomentMatching`).

**Example:** [`example/mc_variance_strategies_example.md`](example/mc_variance_strategies_example.md)


## Build & Run

Prerequisites: C++20 compiler (clang++/g++), Boost headers for normal CDF/PDF implementations.

```bash
brew install boost
mkdir -p output
c++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') -o output/main
```

## Future Development
- Greek in MC with various variance reduction method like likelihood
- Other path dependent exotic option pricing engine

## Historical README

The previous long-form README has been archived under "Appendix — Historical README Snapshot (pre-cleanup)" in `RELEASE_NOTES.md`. This keeps the same information available while keeping the project root documentation concise.

## License

OptionPricer is distributed under the terms of the [MIT License](LICENSE).

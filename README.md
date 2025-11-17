# OptionPricer

A high-performance C++ library for pricing financial derivatives, specifically European and American vanilla options using multiple numerical methods.

## Overview

OptionPricer implements several state-of-the-art option pricing algorithms:

| Engine | European | American | Greeks |
|--------|:--------:|:--------:|:------:|
| **Black-Scholes Analytical** | ✓ | ✗ | ✓ |
| **Binomial CRR** | ✓ | ✓ | ✓ |
| **Trinomial Tree** | ✓ | ✓ | ✓ |
| **Monte Carlo European** | ✓ | ✗ | ✗ |

**Pricing Outputs:** All engines return `PriceOutputs` containing the option value and Greeks (Delta, Gamma, Vega, Theta, Rho) where applicable. Monte Carlo additionally provides standard deviation and standard error estimates.

**Design Notes**

*Architecture & Polymorphism*
- The code uses an abstract base class `PricingEngine` which declares a virtual method `price(const core::OptionSpec&, const core::OptionParams&)`.
- Concrete engines (`BSEuropeanAnalytic`, `BinomialCRREngine`, `TrinomialTreeEngine`, `MCEuropeanEngine`) inherit from `PricingEngine` and override `price()`.

*Why C++?*
- **Performance-Critical Workload:** Derivative pricing, especially Monte Carlo simulation, is computationally intensive. C++ offers near-native execution speed without runtime overhead, unlike Python which requires garbage collection and has inherent interpreter latency.
- **Memory Efficiency:** C++ provides explicit memory control and stack allocation, eliminating the garbage collection pauses that Python imposes. For MC simulations generating hundreds of thousands of price paths, this translates to significantly faster execution with predictable latency.
- **Industry Standard:** C++ is the lingua franca in quantitative finance and derivative pricing. Most production pricing engines across investment banks and hedge funds are written in C++, making it the de facto choice for high-frequency computational finance.

*Mathematical Libraries*
- Leverages **Boost C++ Libraries** (`boost::math::distributions::normal`) for robust, well-tested implementations of the standard normal cumulative distribution function (CDF) and probability density function (PDF), avoiding hand-rolled approximations and ensuring numerical accuracy.

## Project Structure

```
OptionPricer/
├── example/                           # Demo application showcasing all engines
│   └── example.cpp                    # Demo application (moved from main.cpp)
├── src/
│   ├── core/
│   │   └── Types.hpp                  # Core data structures (OptionSpec, OptionParams, etc.)
│   ├── engines/
│   │   ├── PricingEngine.hpp          # Abstract base class for all engines
│   │   ├── BSEuropeanAnalytic.{hpp,cpp}     # Black-Scholes analytical pricing
│   │   ├── BinomialCRR.{hpp,cpp}            # Binomial tree engine (CRR model)
│   │   ├── TrinomialTree.{hpp,cpp}          # Trinomial tree engine
│   │   └── MCEuropean.{hpp,cpp}             # Monte Carlo engine for European options
│   └── math/
│       ├── Normal.{hpp,cpp}           # Standard normal distribution utilities
│       ├── Stats.{hpp,cpp}            # Statistical helpers (mean, variance, etc.)
├── output/
│   └── example                        # Compiled executable (was "main")
└── README.md                          # This file
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

## Pricing Methodologies

### 1. Black-Scholes Analytical (BSEuropeanAnalytic)

**Applicability:** European vanilla options only

**Method:** Closed-form solution using the famous Black-Scholes formula:

$$C = S_0 e^{-qT} N(d_1) - K e^{-rT} N(d_2)$$

where:
- $d_1 = \frac{\ln(S_0/K) + (r - q + \sigma^2/2)T}{\sigma\sqrt{T}}$
- $d_2 = d_1 - \sigma\sqrt{T}$
- $N(\cdot)$ is the cumulative standard normal distribution

**Greeks:** Computed analytically using closed-form derivatives.

**Advantages:**
- Instantaneous computation
- Exact for European options (no discretization error)
- Analytical Greeks available

**Disadvantages:**
- Cannot price American options
- Assumes constant volatility and rates
- No dividend adjustments beyond the q parameter

### 2. Binomial CRR (BinomialCRREngine)

**Applicability:** Both European and American vanilla options

**Method:** Constructs a discrete recombining binomial tree over $[0, T]$ with $n$ steps.

At each node:
- Up move: $u = e^{\sigma\sqrt{\Delta t}}$
- Down move: $d = 1/u$
- Risk-neutral probability: $p = \frac{e^{(r-q)\Delta t} - d}{u - d}$

At maturity ($t = T$), payoff = intrinsic value at each leaf node.

Backwards induction from maturity to present:
- **European:** $V = e^{-r\Delta t} [p \cdot V_u + (1-p) \cdot V_d]$
- **American:** $V = \max(\text{intrinsic}, e^{-r\Delta t}[\cdots])$ (early exercise check)

**Greeks:** Computed via finite differences:
- Delta: $\frac{V(S + h) - V(S - h)}{2h}$
- Gamma: $\frac{V(S+h) - 2V(S) + V(S-h)}{h^2}$

**Advantages:**
- Handles American options (early exercise)
- Simple to understand and implement
- Greeks via finite differences
- Converges to Black-Scholes as $n \to \infty$

**Disadvantages:**
- Slow convergence (order $O(n^{-1})$)
- Requires many steps for accuracy
- Numerical Greeks less stable than analytical

### 3. Trinomial Tree (TrinomialTreeEngine)

**Applicability:** Both European and American vanilla options

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

**Greeks:** Computed via finite differences (same as binomial).

**Advantages:**
- Faster convergence than binomial (order $O(n^{-2})$)
- Fewer steps needed for accuracy
- Handles American options
- More stable Greeks estimation

**Disadvantages:**
- Slightly more complex than binomial
- Still slower than analytical methods

### 4. Monte Carlo European (MCEuropeanEngine)

**Applicability:** European vanilla options only

**Method:** Stochastic simulation under the risk-neutral measure:

$$dS = (r - q)S\,dt + \sigma S\,dW$$

Discretization (Euler):
$$S_{i+1} = S_i e^{(r-q-\sigma^2/2)\Delta t + \sigma\sqrt{\Delta t} Z_i}$$

where $Z_i \sim N(0,1)$.

1. Simulate $M$ paths of stock prices from $t=0$ to $t=T$
2. Compute payoff at maturity for each path
3. Discount back: $V = e^{-rT} \times \text{mean(payoffs)}$
4. Estimate standard error from sample variance

**Advantages:**
- Very flexible (easily extended to exotic options)
- No discretization in underlying (only in time)
- Parallelizable

**Disadvantages:**
- Slow convergence (order $O(M^{-1/2})$)
- Requires many paths for accuracy
- Cannot price American options (path-dependent logic harder)
- Statistical noise in results

## Building & Running

### Prerequisites

- **macOS/Linux** with Xcode Command Line Tools or GCC
- **Boost C++ Libraries** (for `boost::math::distributions::normal`)
  ```bash
  brew install boost
  ```

### Quick Build

From the project root:
```bash
mkdir -p output && g++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') -o output/main
```

To build the example application:
```bash
mkdir -p output && g++ -std=c++20 -O2 -I"$(brew --prefix boost)/include" $(find ./src -name '*.cpp') ./example/example.cpp -o output/example
```

### Run Demo
```bash
./output/example
```

### Using the Task System (VS Code)

1. Press `Cmd + Shift + B` to run the default build task
2. Build output goes to `output/main`
3. For example: `./output/main`

## Usage Example

```cpp
#include "core/Types.hpp"
#include "engines/BSEuropeanAnalytic.hpp"
#include "engines/BinomialCRR.hpp"

int main() {
    // Define market parameters
    core::OptionParams params{
        100.0,   // S: spot price
        100.0,   // K: strike price
        0.05,    // r: 5% risk-free rate
        0.02,    // q: 2% dividend yield
        0.20,    // sig: 20% volatility
        1.0      // T: 1 year to maturity
    };

    // Define a European call option
    core::OptionSpec call_euro{
        {params.K, core::OptionType::Call},           // Payoff: max(S-K, 0)
        core::ExerciseStyle::European
    };

    // Price using Black-Scholes
    engines::BSEuropeanAnalytic bs_engine;
    auto bs_result = bs_engine.price(call_euro, params);
    
    std::cout << "BS Call Price: " << bs_result.value << std::endl;
    std::cout << "Delta: " << bs_result.delta << std::endl;
    std::cout << "Gamma: " << bs_result.gamma << std::endl;

    // Price using Binomial (works for American too)
    engines::BinomialCRREngine binom_engine(4000);  // 4000 steps
    auto binom_result = binom_engine.price(call_euro, params);
    
    std::cout << "Binomial Call Price: " << binom_result.value << std::endl;

    // American put (only Binomial/Trinomial work)
    core::OptionSpec put_amer{
        {params.K, core::OptionType::Put},
        core::ExerciseStyle::American
    };
    auto amer_result = binom_engine.price(put_amer, params);
    std::cout << "American Put Price: " << amer_result.value << std::endl;

    return 0;
}
```

## Demo Output Example

Running `./output/example` produces:

```text
Market Parameters: S=100, K=100, r=5%, q=2%, σ=20%, T=1 year

CALL OPTION:
             Black-Scholes   | Value:   9.227006  Delta:   0.586851  Gamma:   0.018951
         Binomial (2000)   | Value:   9.226034  Delta:   0.586841  Gamma:   0.338956
        Trinomial (2000)   | Value:   9.226052  Delta:   0.586841  Gamma:   0.293581
     Monte Carlo (50k)   | Value:   9.223152  StdDev:  13.869651  StdErr:   0.062027

PUT OPTION:
             Black-Scholes   | Value:   6.330081  Delta:  -0.393348  Gamma:   0.018951
         Binomial (2000)   | Value:   6.329109  Delta:  -0.393357  Gamma:   0.338956
        Trinomial (2000)   | Value:   6.329129  Delta:  -0.393358  Gamma:   0.293581
     Monte Carlo (50k)   | Value:   6.323574  StdDev:   9.127413  StdErr:   0.040819

========== SCENARIO 2: AMERICAN OPTIONS ==========

Market Parameters: S=100, K=100, r=5%, q=0% (no dividend), σ=20%, T=1 year

AMERICAN CALL (European worth same without dividend):
         Binomial (2000)   | Value:  10.449584  Delta:   0.636817  Gamma:   0.335583
        Trinomial (2000)   | Value:  10.449589  Delta:   0.636816  Gamma:   0.290662
Monte Carlo (50k) Euro | Value:  10.446425  StdDev:  14.756659  StdErr:   0.065994

AMERICAN PUT (early exercise premium shown):
             Binomial Euro   | Value:   5.572526  Delta:  -0.363183  Gamma:   0.335583
             Binomial Amer   | Value:   6.089990  Delta:  -0.411874  Gamma:   0.189997
    Early exercise premium (Binomial): 0.517464
            Trinomial Euro   | Value:   5.572554  Delta:  -0.363184  Gamma:   0.290662
            Trinomial Amer   | Value:   6.089446  Delta:  -0.411410  Gamma:   0.181079
Monte Carlo (50k) Euro | Value:   5.566661  StdDev:   8.618131  StdErr:   0.038541
    Early exercise premium (Trinomial): 0.516893

========== SCENARIO 3: BINOMIAL vs TRINOMIAL CONVERGENCE ==========

Convergence to Black-Scholes (European Call):
Reference BS Price: 9.227006

Steps     Binomial Error      Trinomial Error
------------------------------------------------
25000     0.00007776          0.00007630
50000     0.00003888          0.00003815
75000     0.00002592          0.00002543
100000    0.00001944          0.00001907
```

**Notes:**
- Trinomial convergence: The trinomial tree converges faster than the binomial CRR tree toward the Black–Scholes reference price for European options. The trinomial discretization more accurately matches the first three moments of the continuous-time process per time-step (it uses an up/mid/down move and tuned probabilities), which reduces local truncation error versus a two-point approximation. This produces a higher-order convergence in practice, so fewer steps are needed to reach the same accuracy.
- log bump: all Greeks in the tree engines use a multiplicative log bump for scale-invariant purpose. For a bump parameter `b` we use: `S_up = S * exp(+b)` and `S_down = S * exp(-b)`
    - Delta is computed as `(V(S_up) - V(S_down)) / (S_up - S_down)`
    - Gamma is computed as `2 * ( (V_up - V0) / (S_up - S) - (V0 - V_down) / (S - S_down) ) / (S_up - S_down)`

**v2.0.0 Staging (Current)**

The project structure has been reorganized for v2.0.0:
- Moved example/demo code from `src/main.cpp` to `example/example.cpp`
- Separated library code (in `src/`) from demonstration code
- This enables the library to be used as a standalone component without the demo application
- Future releases will support installation as a standalone library package

**v2.0.0 Roadmap (Planned Enhancements)**

- Add Least-Squares Monte Carlo (LSMC) to price American options via regression on pathwise continuation values (works within the existing MC framework).
- Add additional Monte Carlo variance-reduction techniques and frameworks:
    - Control variates (e.g., use analytic European price as control)
    - Antithetic variates
    - Importance sampling / stratified sampling
    - Quasi-Monte Carlo (Sobol, Halton) for lower-discrepancy sampling
    - Multi-level Monte Carlo (MLMC) experimentation
- Exotic options pricing via MC: Barrier, Asian, lookback 
- Calibration: Implied vol extraction from market prices
- Library installation and packaging support

## References

- Black, F., & Scholes, M. (1973). "The pricing of options and corporate liabilities." *Journal of Political Economy*.
- Cox, J. C., Ross, S. A., & Rubinstein, M. (1979). "Option pricing: A simplified approach." *Journal of Financial Economics*.
- Hull, J. C. (2018). *Options, Futures, and Other Derivatives* (10th ed.).
- Glasserman, P. (2003). *Monte Carlo Methods in Financial Engineering*.
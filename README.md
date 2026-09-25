# OptionPricer

A C++20 derivatives pricing project covering analytical formulas, lattice methods, and Monte Carlo simulation.

**Status:** C++20 engineering project with worked examples and 11 automated numerical regression cases. CI builds and tests on Linux and macOS. This is not a production-validated pricing library.

[![Numerical validation](https://github.com/SRG-eddieliu/OptionPricer/actions/workflows/numerical.yml/badge.svg)](https://github.com/SRG-eddieliu/OptionPricer/actions/workflows/numerical.yml)

## Implemented Methods

| Method | Scope | Worked example |
| --- | --- | --- |
| Black-Scholes | European vanilla prices and analytical Greeks | [Analytical pricing](example/black_scholes_example.md) |
| Cox-Ross-Rubinstein tree | European and American vanilla options | [Binomial tree](example/binomial_example.md) |
| Trinomial tree | European and American vanilla options | [Trinomial tree](example/trinomial_example.md) |
| European Monte Carlo | Simulated vanilla payoffs | [European MC](example/mc_european_example.md) |
| Longstaff-Schwartz Monte Carlo | American exercise using continuation-value regression | [American LSMC](example/mc_american_lsmc_example.md) |
| Path-dependent Monte Carlo | Arithmetic Asian, barrier, and lookback payoffs | [Path-dependent options](example/mc_path_exotics_example.md) |
| Variance reduction | Antithetic variates and moment matching | [Variance reduction](example/mc_variance_strategies_example.md) |

## Build and Run

Requires CMake 3.20 or newer, a C++20 compiler, and Boost headers.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/option_pricer_demo
./build/black_scholes_example
```

Other example executables follow the source filenames in [example/](example), such as `binomial_example` and `mc_american_lsmc_example`.

On macOS, Boost can be installed with `brew install boost`. The existing [build script](scripts/build_main.sh) provides an alternative to CMake.

## Design

- [Contract and market types](src/core/Types.hpp) separate payoff, exercise style, and market inputs.
- [PricingEngine](src/engines/PricingEngine.hpp) defines the shared vanilla pricing interface.
- [Pricing engines](src/engines) contain method-specific implementations and shared Monte Carlo infrastructure.
- [Numerical helpers](src/math) provide normal-distribution and statistics utilities.
- [Examples](example) pair executable demonstrations with method notes.

The project is intended to make numerical methods and implementation tradeoffs inspectable. Example outputs are demonstrations, not a substitute for independent accuracy and convergence checks.

## Numerical Validation

The [Boost.Test suite](tests/numerical_tests.cpp) checks analytical reference prices,
put-call parity, price bounds, finite-difference Greeks, tree convergence,
American/European consistency, zero-volatility/expiry behavior, invalid inputs,
eight-seed European MC error bounds, 32-seed variance-reduction comparisons,
LSMC's time-zero decision, an American-put tree reference, and barrier in/out parity.

These tests exposed and now protect fixes for deterministic discounting, silent
tree-probability clipping, and a pathwise time-zero exercise decision in LSMC.
Moment-matched paths and in-sample LSMC do not support the naive IID standard-error
formula; their `std_error` is now `NaN`, not a purported confidence interval.

See [validation scope and conventions](docs/validation.md) for tolerances,
parameter restrictions, unimplemented Greeks, and remaining limitations.

## Further Reading

- [LSMC reference workbook](reference/LSMC%20replication.xlsx)
- [Release notes and historical documentation](RELEASE_NOTES.md)

## License

[MIT](LICENSE).

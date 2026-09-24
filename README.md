# OptionPricer

A C++20 derivatives pricing project covering analytical formulas, lattice methods, and Monte Carlo simulation.

**Status:** engineering project with CMake targets and worked examples. A comprehensive automated numerical regression suite is still planned; this is not a production-validated pricing library.

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

## Validation Priorities

- Compare vanilla prices and Greeks against analytical benchmarks and finite-difference checks.
- Test tree convergence and American/European consistency cases.
- Check Monte Carlo confidence intervals and variance-reduction behavior across seeds.
- Validate LSMC estimates against independent reference values and inspect regression stability.
- Test boundary cases, invalid inputs, and discretization sensitivity for path-dependent payoffs.

These items describe planned validation work, not tests already passing in this repository.

## Further Reading

- [LSMC reference workbook](reference/LSMC%20replication.xlsx)
- [Release notes and historical documentation](RELEASE_NOTES.md)

## License

[MIT](LICENSE).

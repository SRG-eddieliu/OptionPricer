# Numerical Validation and Conventions

Run `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, `cmake --build build`, then
`ctest --test-dir build --output-on-failure`. The suite uses header-only Boost.Test.
CTest also runs the European MC example with a timeout: its seed is explicitly
the third constructor argument, not the time-step count. There are 13 numerical
test groups plus this example smoke test.

## What the tests establish

| Check | Scope / tolerance |
| --- | --- |
| Analytical reference | S=K=100, r=5%, q=0, sigma=20%, T=1: call 10.4505835722, put 5.5735260223; absolute tolerance 1e-10 |
| Parity and bounds | Calls/puts, nonzero dividends, positive and negative rates |
| Greeks | Central finite differences: delta/gamma 1e-7, vega/theta 1e-6, rho 1e-5 |
| Trees | 100 vs 800 steps; fine-grid price error below 0.005 (CRR) / 0.006 (trinomial) in the specified case; no universal monotone convergence claim |
| Tree Greeks | Three parameter sets, calls/puts, 100 vs 800 steps: fine-grid gamma absolute error <1e-4 and delta <5e-4; default 4,000-step ATM gamma error <2e-5; sub-grid bumps and non-dividend American calls covered |
| Exercise | Non-dividend American call matches European tree; American put at least European and intrinsic |
| European MC | Eight fixed seeds, 40,000 paths each; error under five estimated standard errors; this is a regression guard, not a calibration study of 95% coverage |
| Variance reduction | Across 32 seeds at equal path budget, antithetic and moment-matching estimates have lower variance in the specified vanilla call case; not guaranteed for every payoff |
| LSMC | Single-step decision equals max(intrinsic, average continuation); mean of three 30,000-path estimates within 0.22 of a 1,000-step American-put CRR reference |
| Barrier | Same simulated paths: knock-in + knock-out equals vanilla; discrete monitoring includes initial spot |

## Corrections protected by regression tests

- Zero volatility uses the discounted payoff at the deterministic terminal spot;
  American zero-volatility exercise is evaluated on the engine's time grid.
- Time-zero exercise is one decision based on average continuation, not the
  average of pathwise maxima (which used future information).
- Invalid tree probabilities now fail with a request to refine the grid rather
  than silently changing the risk-neutral distribution.
- Spot and both strike fields must be positive and finite; the two strike fields
  must agree. Volatility/maturity cannot be negative. Rates/yields must be finite.
- MC requires at least two paths and one step; antithetic sampling needs at least
  four paths and an even count. QMC and multilevel enum values are not implemented
  and now raise instead of silently running ordinary MC.

## Tree Greek mesh policy

A fixed tree price is piecewise linear in spot below terminal payoff-node spacing.
A tiny spot bump can therefore produce a large, nonconvergent gamma at a kink.
Tree delta/gamma now use a log-spot bump at least equal to terminal-node spacing:
`2 * sigma * sqrt(T / steps)` for CRR and `sigma * sqrt(3*T / steps)` for the
trinomial tree. The constructor bump is a lower bound, not an exact stencil size.
Prices are unchanged; the existing nonuniform central-difference formula uses
these resolved spot distances. Zero-volatility and expiry paths retain the
requested bump. A nonpositive bump retains the legacy behavior of disabling
Greek calculations.

At S=K=100, r=5%, q=0, sigma=20%, T=1, the default 4,000-step gamma is checked
against analytical Black-Scholes. The parameter-grid cases also test dividends,
moneyness, maturities, calls and puts. These are numerical regression cases,
not a universal error bound, a guarantee near an American exercise boundary,
or validation of unsupported Greek fields. Refine the time grid and inspect
sensitivity for new parameter regions, especially short maturities or large
requested bumps.

## Output interpretation and remaining work

- Greeks are per unit change (not per 1%); theta is calendar-time decay per year.
  Only Black-Scholes provides all listed Greeks. Trees approximate delta/gamma;
  other unsupported Greek fields retain legacy zero defaults and are **not**
  valid zero-risk estimates. Black-Scholes boundary-case Greeks return NaN.
- For antithetic-only MC, sample statistics use independent **pair means**.
  `std_dev` therefore describes those estimator observations, not individual paths.
- Moment matching couples all draws. Its naive standard error is unavailable
  (NaN); uncertainty requires independent replications. LSMC also returns NaN for
  standard error because policy fitting and valuation share paths. Its reported
  cash-flow standard deviation is descriptive, not a pricing confidence interval.
- LSMC still needs independent policy evaluation / cross-fitting, more parameter
  cases, and regression-conditioning diagnostics. A tree comparison does not
  prove an unbiased estimator or a production-ready American engine.
- Asian averaging and lookbacks include time zero; barriers use discrete time
  monitoring, with no Brownian-bridge correction. Full exotic accuracy and mesh
  sensitivity studies remain future work.
- C++ normal draws need not be bitwise identical between standard libraries.
  Stochastic checks use tolerances; these fixed-seed cases are regression tests.

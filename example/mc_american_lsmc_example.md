# American LSMC Monte Carlo Example

Longstaff–Schwartz (Laguerre basis degree 3) American put valuation compared against a binomial reference.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/mc_american_lsmc_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/mc_american_lsmc_example
```

## Run

```bash
./output/mc_american_lsmc_example
```

## Output

```
American Put via LSMC (Laguerre basis degree 3) vs Binomial reference
Binomial Reference | Value: 6.090181
  LSMC (75k paths) | Value:   6.124205  StdDev:   7.265955  StdErr:   0.026532
```

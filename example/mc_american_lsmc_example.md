# American LSMC Monte Carlo Example

Longstaff–Schwartz (Laguerre basis degree 3) American call/put valuation compared against Black–Scholes European baselines and binomial American references.

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
American options via Longstaff-Schwartz (Laguerre basis degree 3)

American Call (should align with European baseline):
Black-Scholes Euro baseline | Value: 10.450584
Binomial American reference | Value: 10.450084
      LSMC (50000) | Value:  10.333223  StdDev:  14.724956  StdErr:   0.065852
      LSMC (75000) | Value:  10.402288  StdDev:  14.663180  StdErr:   0.053542
     LSMC (100000) | Value:  10.433339  StdDev:  14.725789  StdErr:   0.046567

American Put (early exercise premium vs binomial):
Black-Scholes Euro baseline | Value: 5.573526
Binomial American reference | Value: 6.090181
      LSMC (50000) | Value:   6.074838  StdDev:   7.041625  StdErr:   0.031491
      LSMC (75000) | Value:   6.068655  StdDev:   7.141309  StdErr:   0.026076
     LSMC (100000) | Value:   6.098410  StdDev:   7.133357  StdErr:   0.022558
```

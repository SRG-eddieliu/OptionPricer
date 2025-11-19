# Monte Carlo Variance Strategies Example

Unified demo showing plain MC, antithetic variates, moment matching, and the combined approach for both a European call (with Black–Scholes baseline) and an American put (with binomial baseline).

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/mc_variance_strategies_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/mc_variance_strategies_example
```

## Run

```bash
./output/mc_variance_strategies_example
```

## Output

```
European Call Monte Carlo Variance Strategies
Params: S=120, K=110, r=0.02, q=0, sigma=0.2, T=1
Black-Scholes baseline: 16.425707

-- Paths: 30000 --
                        Plain MC | Value:  16.455388  StdDev:  19.630590  StdErr:   0.113337
                 MC + Antithetic | Value:  16.352930  StdDev:   8.192834  StdErr:   0.066894
            MC + Moment Matching | Value:  16.428274  StdDev:  19.587714  StdErr:   0.113090
          MC + Antithetic+Moment | Value:  16.391914  StdDev:   8.073987  StdErr:   0.065924

-- Paths: 60000 --
                        Plain MC | Value:  16.439780  StdDev:  19.481918  StdErr:   0.079535
                 MC + Antithetic | Value:  16.347902  StdDev:   8.101581  StdErr:   0.046774
            MC + Moment Matching | Value:  16.450295  StdDev:  19.469623  StdErr:   0.079484
          MC + Antithetic+Moment | Value:  16.418153  StdDev:   8.165428  StdErr:   0.047143

-- Paths: 90000 --
                        Plain MC | Value:  16.398184  StdDev:  19.535876  StdErr:   0.065120
                 MC + Antithetic | Value:  16.436574  StdDev:   8.205664  StdErr:   0.038682
            MC + Moment Matching | Value:  16.423320  StdDev:  19.556966  StdErr:   0.065190
          MC + Antithetic+Moment | Value:  16.429536  StdDev:   8.216746  StdErr:   0.038734

American Put via LSMC (variance strategies)
Params: S=100.000000, K=100.000000, r=0.040000, q=0.000000, sigma=0.250000, T=1.000000
Binomial baseline: 8.312846

-- Paths: 50000 --
                        Plain MC | Value:   8.257332  StdDev:   9.328756  StdErr:   0.041719
                 MC + Antithetic | Value:   8.299435  StdDev:   3.705324  StdErr:   0.023435
            MC + Moment Matching | Value:   8.319611  StdDev:   9.482951  StdErr:   0.042409
          MC + Antithetic+Moment | Value:   8.328854  StdDev:   3.768191  StdErr:   0.023832

-- Paths: 100000 --
                        Plain MC | Value:   8.291277  StdDev:   9.471701  StdErr:   0.029952
                 MC + Antithetic | Value:   8.261690  StdDev:   3.733885  StdErr:   0.016698
            MC + Moment Matching | Value:   8.288750  StdDev:   9.470444  StdErr:   0.029948
          MC + Antithetic+Moment | Value:   8.284281  StdDev:   3.758617  StdErr:   0.016809

-- Paths: 150000 --
                        Plain MC | Value:   8.259219  StdDev:   9.431491  StdErr:   0.024352
                 MC + Antithetic | Value:   8.290188  StdDev:   3.739566  StdErr:   0.013655
            MC + Moment Matching | Value:   8.262553  StdDev:   9.376123  StdErr:   0.024209
          MC + Antithetic+Moment | Value:   8.289309  StdDev:   3.723381  StdErr:   0.013596
```

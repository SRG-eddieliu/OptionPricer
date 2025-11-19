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
                 MC+Antithetic | Value:  16.352930  StdDev:   8.192834  StdErr:   0.066894
            MC+Moment Matching | Value:  16.428274  StdDev:  19.587714  StdErr:   0.113090
          MC+Antithetic+Moment | Value:  16.391914  StdDev:   8.073987  StdErr:   0.065924

-- Paths: 60000 --
                        Plain MC | Value:  16.439780  StdDev:  19.481918  StdErr:   0.079535
                 MC+Antithetic | Value:  16.347902  StdDev:   8.101581  StdErr:   0.046774
            MC+Moment Matching | Value:  16.450295  StdDev:  19.469623  StdErr:   0.079484
          MC+Antithetic+Moment | Value:  16.418153  StdDev:   8.165428  StdErr:   0.047143

American Put via LSMC (variance strategies)
Params: S=100.000000, K=100.000000, r=0.040000, q=0.000000, sigma=0.250000, T=1.000000
Binomial baseline: 8.312846

-- Paths: 40000 --
                        Plain MC | Value:   8.226420  StdDev:   9.447481  StdErr:   0.047237
                 MC+Antithetic | Value:   8.292804  StdDev:   3.767204  StdErr:   0.026638
            MC+Moment Matching | Value:   8.274790  StdDev:   9.472750  StdErr:   0.047364
          MC+Antithetic+Moment | Value:   8.296362  StdDev:   3.692394  StdErr:   0.026109

-- Paths: 80000 --
                        Plain MC | Value:   8.269923  StdDev:   9.465650  StdErr:   0.033466
                 MC+Antithetic | Value:   8.276971  StdDev:   3.691688  StdErr:   0.018458
            MC+Moment Matching | Value:   8.290984  StdDev:   9.396055  StdErr:   0.033220
          MC+Antithetic+Moment | Value:   8.287711  StdDev:   3.775314  StdErr:   0.018877
```

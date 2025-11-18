# European Monte Carlo Example

Monte Carlo valuation of a European call showing sample mean, standard deviation, and standard error, compared against the Black–Scholes analytic baseline.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/mc_european_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/mc_european_example
```

## Run

```bash
./output/mc_european_example
```

## Output

```
European Monte Carlo pricing (call) for S=120, K=110, r=2%, q=0%, sigma=15%, T=2
Black-Scholes Call baseline: 18.338750
MC (50000 paths) | Value:  18.295371  StdDev:  21.222572  StdErr:   0.094910
MC (75000 paths) | Value:  18.219424  StdDev:  21.101926  StdErr:   0.077053
MC (100000 paths) | Value:  18.402411  StdDev:  21.264241  StdErr:   0.067243
```

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
European Monte Carlo pricing for S=120, K=110, r=2%, q=0%, sigma=15%, T=2 (50k paths)
Black-Scholes baseline (analytic): 18.338750
Value:  18.339413  StdDev:  21.274422  StdErr:   0.095142
```

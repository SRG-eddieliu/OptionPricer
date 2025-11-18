# Monte Carlo Variance Reduction Example

Demonstrates antithetic-path variance reduction versus plain Monte Carlo on a European call, alongside the Black–Scholes baseline.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/mc_variance_reduction_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/mc_variance_reduction_example
```

## Run

```bash
./output/mc_variance_reduction_example
```

## Output

```
European Call Monte Carlo — Variance Reduction Comparison
Params: S=100, K=100, r=0.01, q=0, sigma=0.25, T=1
Black-Scholes Call baseline: 10.403539

-- Paths: 25000 --
                    Plain MC | Value:  10.337680  StdDev:  17.162274  StdErr:   0.108544
             MC + Antithetic | Value:  10.263233  StdDev:   9.491298  StdErr:   0.084893

-- Paths: 50000 --
                    Plain MC | Value:  10.379572  StdDev:  17.265319  StdErr:   0.077213
             MC + Antithetic | Value:  10.320724  StdDev:   9.659854  StdErr:   0.061094

-- Paths: 100000 --
                    Plain MC | Value:  10.440370  StdDev:  17.333658  StdErr:   0.054814
             MC + Antithetic | Value:  10.476667  StdDev:   9.884245  StdErr:   0.044204
```

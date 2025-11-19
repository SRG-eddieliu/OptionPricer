# Path-Dependent Monte Carlo Example

Demonstrates the `MCPathDependentEngine` pricing arithmetic Asian, barrier, and lookback options. Scenario A uses 60k paths/90 steps; Scenario B uses 120k paths/180 steps. Inputs:

- **Arithmetic Asian Call:** `S=100`, `K=95`, `r=1.5%`, `q=0%`, `σ=20%`, `T=1.0`
- **Down-and-Out Put:** `S=120`, `K=115`, `barrier=100 (down-and-out)`, `r=2%`, `σ=25%`, `T=0.75`
- **Lookback Call:** `S=90`, `K=85`, `r=1%`, `σ=30%`, `T=1.25`

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/mc_path_exotics_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/mc_path_exotics_example
```

## Run

```bash
./output/mc_path_exotics_example
```

## Output

```
Path-Dependent Monte Carlo Examples
Scenario A: 60k paths, 90 steps
       Arithmetic Asian Call | Value:   7.819601  StdDev:   9.011822  StdErr:   0.036791
            Down-and-Out Put | Value:   0.634581  StdDev:   2.202014  StdErr:   0.008990
               Lookback Call | Value:  29.793519  StdDev:  24.403238  StdErr:   0.099626

Scenario B: 120k paths, 180 steps
       Arithmetic Asian Call | Value:   7.796005  StdDev:   9.044901  StdErr:   0.026110
            Down-and-Out Put | Value:   0.583594  StdDev:   2.096536  StdErr:   0.006052
               Lookback Call | Value:  30.414153  StdDev:  24.701050  StdErr:   0.071306
```

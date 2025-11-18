# Binomial vs Trinomial Lattice Example

Evaluates European calls/puts and American calls/puts across the CRR binomial and trinomial engines, with Black–Scholes baselines for the European contracts.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/binomial_trinomial_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/binomial_trinomial_example
```

## Run

```bash
./output/binomial_trinomial_example
```

## Output

```
Binomial vs. Trinomial lattice pricing for S=95, K=100, r=4%, sigma=20%, T=1

Black-Scholes baseline (European only):
           BS Call | Value:   7.082878  Delta:   0.517362  Gamma:   0.020977
            BS Put | Value:   8.161822  Delta:  -0.482638  Gamma:   0.020977

European Call comparison:
          Binomial | Value:   7.083314  Delta:   0.521564  Gamma:   0.000000
         Trinomial | Value:   7.083243  Delta:   0.519245  Gamma:  -0.000000

European Put comparison:
          Binomial | Value:   8.162257  Delta:  -0.478436  Gamma:  -0.000000
         Trinomial | Value:   8.162197  Delta:  -0.480755  Gamma:  -0.000000

American Call (should match European without dividends):
          Binomial | Value:   7.083314  Delta:   0.521564  Gamma:   0.000000
         Trinomial | Value:   7.083243  Delta:   0.519245  Gamma:  -0.000000

American Put (early exercise premium highlighted):
          Binomial | Value:   8.787196  Delta:  -0.534999  Gamma:   0.014678
         Trinomial | Value:   8.786636  Delta:  -0.536398  Gamma:   0.012780
  Early exercise premium (Binomial) : 0.624938
  Early exercise premium (Trinomial): 0.624440
```

# Trinomial Tree Example

Trinomial lattice pricing for European and American contracts with Black–Scholes baselines.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/trinomial_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/trinomial_example
```

## Run

```bash
./output/trinomial_example
```

## Output

```
Trinomial tree pricing for S=95, K=100, r=4%, sigma=20%, T=1

Black-Scholes baseline (European only):
           BS Call | Value:   7.082878  Delta:   0.517362  Gamma:   0.020977
            BS Put | Value:   8.161822  Delta:  -0.482638  Gamma:   0.020977

European Call (Trinomial vs BS):
         Trinomial | Value:   7.083243  Delta:   0.519245  Gamma:  -0.000000

European Put (Trinomial vs BS):
         Trinomial | Value:   8.162197  Delta:  -0.480755  Gamma:  -0.000000

American Call (should match European without dividends):
         Trinomial | Value:   7.083243  Delta:   0.519245  Gamma:  -0.000000

American Put (early exercise premium highlighted):
         Trinomial | Value:   8.786636  Delta:  -0.536398  Gamma:   0.012780
  Early exercise premium: 0.624440
```

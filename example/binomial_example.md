# Binomial Tree Example

Cox–Ross–Rubinstein binomial pricing with European and American contracts plus Black–Scholes baselines.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/binomial_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/binomial_example
```

## Run

```bash
./output/binomial_example
```

## Output

```
Cox-Ross-Rubinstein binomial pricing for S=95, K=100, r=4%, sigma=20%, T=1

Black-Scholes baseline (European only):
           BS Call | Value:   7.082878  Delta:   0.517362  Gamma:   0.020977
            BS Put | Value:   8.161822  Delta:  -0.482638  Gamma:   0.020977

European Call (Binomial vs BS):
          Binomial | Value:   7.083314  Delta:   0.521564  Gamma:   0.000000

European Put (Binomial vs BS):
          Binomial | Value:   8.162257  Delta:  -0.478436  Gamma:  -0.000000

American Call (should match European without dividends):
          Binomial | Value:   7.083314  Delta:   0.521564  Gamma:   0.000000

American Put (early exercise premium highlighted):
          Binomial | Value:   8.787196  Delta:  -0.534999  Gamma:   0.014678
  Early exercise premium: 0.624938
```

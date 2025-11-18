# Binomial vs Trinomial Lattice Example

Evaluates European calls and American puts across the CRR binomial and trinomial engines.

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

European Call:
          Binomial | Value:   7.083314  Delta:   0.521564  Gamma:   0.000000
         Trinomial | Value:   7.083243  Delta:   0.519245  Gamma:  -0.000000

American Put:
          Binomial | Value:   8.787196  Delta:  -0.534999  Gamma:   0.014678
         Trinomial | Value:   8.786636  Delta:  -0.536398  Gamma:   0.012780

Early exercise premium (Binomial Put - Euro Call proxy): 0.624938
```

# Black-Scholes Analytic Example

Demonstrates closed-form pricing of a European call/put with Greeks using `BSEuropeanAnalytic`.

## Build

```bash
mkdir -p output
c++ -std=c++20 -O2 -I./src -I"$(brew --prefix boost)/include" example/black_scholes_example.cpp $(find ./src -name '*.cpp' ! -name 'main.cpp') -o output/black_scholes_example
```

## Run

```bash
./output/black_scholes_example
```

## Output

```
Black-Scholes analytic pricing for S=100, K=105, r=3%, q=1%, sigma=25%, T=0.5

      Call | Value:   5.349805  Delta:   0.445637  Gamma:   0.022263  Vega:  27.828817
       Put | Value:   9.285310  Delta:  -0.549375  Gamma:   0.022263  Vega:  27.828817
```

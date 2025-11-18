#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BinomialCRR.hpp"
#include "../src/engines/TrinomialTree.hpp"

namespace {

void print_outputs(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(18) << label << " | Value: " << std::setw(10) << out.value
              << "  Delta: " << std::setw(10) << out.delta << "  Gamma: " << std::setw(10) << out.gamma << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{95.0, 100.0, 0.04, 0.00, 0.20, 1.0};

    engines::BinomialCRREngine binom(2000, 0.0005);
    engines::TrinomialTreeEngine tri(2000, 0.0005);

    core::OptionSpec euro_call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    core::OptionSpec euro_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::European};
    core::OptionSpec amer_put = euro_put;
    amer_put.exercise = core::ExerciseStyle::American;

    auto binom_call = binom.price(euro_call, params);
    auto tri_call = tri.price(euro_call, params);

    auto binom_put = binom.price(amer_put, params);
    auto binom_put_euro = binom.price(euro_put, params);
    auto tri_put = tri.price(amer_put, params);

    std::cout << "Binomial vs. Trinomial lattice pricing for S=95, K=100, r=4%, sigma=20%, T=1\n\n";
    std::cout << "European Call:\n";
    print_outputs("Binomial", binom_call);
    print_outputs("Trinomial", tri_call);

    std::cout << "\nAmerican Put:\n";
    print_outputs("Binomial", binom_put);
    print_outputs("Trinomial", tri_put);

    double premium = binom_put.value - binom_put_euro.value;
    std::cout << "\nEarly exercise premium (Binomial Put - Euro Call proxy): " << std::fixed << std::setprecision(6)
              << premium << '\n';

    return 0;
}

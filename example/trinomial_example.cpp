#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/TrinomialTree.hpp"

namespace {

void print_greeks(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(18) << label << " | Value: " << std::setw(10) << out.value
              << "  Delta: " << std::setw(10) << out.delta << "  Gamma: " << std::setw(10) << out.gamma << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{95.0, 100.0, 0.04, 0.00, 0.20, 1.0};

    engines::BSEuropeanAnalytic bs;
    engines::TrinomialTreeEngine tri(2000, 0.0005);

    core::OptionSpec euro_call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    core::OptionSpec euro_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::European};
    core::OptionSpec amer_call = euro_call;
    amer_call.exercise = core::ExerciseStyle::American;
    core::OptionSpec amer_put = euro_put;
    amer_put.exercise = core::ExerciseStyle::American;

    auto bs_call = bs.price(euro_call, params);
    auto bs_put = bs.price(euro_put, params);

    auto tri_call = tri.price(euro_call, params);
    auto tri_put = tri.price(euro_put, params);
    auto tri_call_amer = tri.price(amer_call, params);
    auto tri_put_amer = tri.price(amer_put, params);

    std::cout << "Trinomial tree pricing for S=95, K=100, r=4%, sigma=20%, T=1\n\n";
    std::cout << "Black-Scholes baseline (European only):\n";
    print_greeks("BS Call", bs_call);
    print_greeks("BS Put", bs_put);

    std::cout << "\nEuropean Call (Trinomial vs BS):\n";
    print_greeks("Trinomial", tri_call);

    std::cout << "\nEuropean Put (Trinomial vs BS):\n";
    print_greeks("Trinomial", tri_put);

    std::cout << "\nAmerican Call (should match European without dividends):\n";
    print_greeks("Trinomial", tri_call_amer);

    std::cout << "\nAmerican Put (early exercise premium highlighted):\n";
    print_greeks("Trinomial", tri_put_amer);
    double premium = tri_put_amer.value - tri_put.value;
    std::cout << "  Early exercise premium: " << std::fixed << std::setprecision(6) << premium << '\n';

    return 0;
}

#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/BinomialCRR.hpp"

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
    engines::BinomialCRREngine binom(2000, 0.0005);

    core::OptionSpec euro_call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    core::OptionSpec euro_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::European};
    core::OptionSpec amer_call = euro_call;
    amer_call.exercise = core::ExerciseStyle::American;
    core::OptionSpec amer_put = euro_put;
    amer_put.exercise = core::ExerciseStyle::American;

    auto bs_call = bs.price(euro_call, params);
    auto bs_put = bs.price(euro_put, params);

    auto binom_call = binom.price(euro_call, params);
    auto binom_put = binom.price(euro_put, params);
    auto binom_call_amer = binom.price(amer_call, params);
    auto binom_put_amer = binom.price(amer_put, params);

    std::cout << "Cox-Ross-Rubinstein binomial pricing for S=95, K=100, r=4%, sigma=20%, T=1\n\n";
    std::cout << "Black-Scholes baseline (European only):\n";
    print_greeks("BS Call", bs_call);
    print_greeks("BS Put", bs_put);

    std::cout << "\nEuropean Call (Binomial vs BS):\n";
    print_greeks("Binomial", binom_call);

    std::cout << "\nEuropean Put (Binomial vs BS):\n";
    print_greeks("Binomial", binom_put);

    std::cout << "\nAmerican Call (should match European without dividends):\n";
    print_greeks("Binomial", binom_call_amer);

    std::cout << "\nAmerican Put (early exercise premium highlighted):\n";
    print_greeks("Binomial", binom_put_amer);
    double premium = binom_put_amer.value - binom_put.value;
    std::cout << "  Early exercise premium: " << std::fixed << std::setprecision(6) << premium << '\n';

    return 0;
}

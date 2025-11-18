#include <iomanip>
#include <iostream>
#include <tuple>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/BinomialCRR.hpp"
#include "../src/engines/MCAmericanLSMC.hpp"

namespace {

void print_mc(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(18) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev << "  StdErr: " << std::setw(10) << out.std_error
              << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{100.0, 100.0, 0.05, 0.00, 0.20, 1.0};
    core::OptionSpec amer_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::American};
    core::OptionSpec amer_call{{params.K, core::OptionType::Call}, core::ExerciseStyle::American};

    core::OptionSpec euro_put = amer_put;
    euro_put.exercise = core::ExerciseStyle::European;
    core::OptionSpec euro_call = amer_call;
    euro_call.exercise = core::ExerciseStyle::European;

    engines::BSEuropeanAnalytic bs;
    engines::BinomialCRREngine binom_ref(4000, 0.0005);
    auto bs_euro_put = bs.price(euro_put, params);
    auto bs_euro_call = bs.price(euro_call, params);
    auto binom_put = binom_ref.price(amer_put, params);
    auto binom_call = binom_ref.price(amer_call, params);

    std::cout << "American options via Longstaff-Schwartz (Laguerre basis degree 3)\n";
    std::cout << std::fixed << std::setprecision(6);

    std::cout << "\nAmerican Call (should align with European baseline):\n";
    std::cout << "Black-Scholes Euro baseline | Value: " << bs_euro_call.value << '\n';
    std::cout << "Binomial American reference | Value: " << binom_call.value << '\n';
    for (auto [paths, steps, seed] : {std::tuple<std::size_t, std::size_t, unsigned int>{50000, 50, 4242},
                                      {75000, 75, 4243},
                                      {100000, 100, 4244}}) {
        engines::MCAmericanLSMCEngine lsmc(paths, steps, seed, 3);
        auto out = lsmc.price(amer_call, params);
        print_mc("LSMC (" + std::to_string(paths) + ")", out);
    }

    std::cout << "\nAmerican Put (early exercise premium vs binomial):\n";
    std::cout << "Black-Scholes Euro baseline | Value: " << bs_euro_put.value << '\n';
    std::cout << "Binomial American reference | Value: " << binom_put.value << '\n';
    for (auto [paths, steps, seed] : {std::tuple<std::size_t, std::size_t, unsigned int>{50000, 50, 5252},
                                      {75000, 75, 5253},
                                      {100000, 100, 5254}}) {
        engines::MCAmericanLSMCEngine lsmc(paths, steps, seed, 3);
        auto out = lsmc.price(amer_put, params);
        print_mc("LSMC (" + std::to_string(paths) + ")", out);
    }

    return 0;
}

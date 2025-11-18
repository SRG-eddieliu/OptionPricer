#include <iomanip>
#include <iostream>

#include "core/Types.hpp"
#include "engines/BinomialCRR.hpp"
#include "engines/MCAmericanLSMC.hpp"

namespace {

void print_mc(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6)
              << std::setw(25) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev
              << "  StdErr: " << std::setw(10) << out.std_error << '\n';
}

void print_ref(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6)
              << std::setw(25) << label << " | Value: " << std::setw(10) << out.value << '\n';
}

}  // namespace

int main() {
    std::cout << "===== LSMC Smoke Test =====\n\n";

    core::OptionParams params{100.0, 100.0, 0.05, 0.00, 0.20, 1.0};
    core::OptionSpec amer_call{{params.K, core::OptionType::Call}, core::ExerciseStyle::American};
    core::OptionSpec amer_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::American};
    core::OptionSpec euro_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::European};

    // Reference lattice engine to compare against
    engines::BinomialCRREngine binom_ref(4000, 0.0005);
    auto binom_call = binom_ref.price(amer_call, params);
    auto binom_put = binom_ref.price(amer_put, params);
    auto binom_put_euro = binom_ref.price(euro_put, params);

    // Baseline LSMC configuration
    engines::MCAmericanLSMCEngine lsmc_50k(50000, 50, 42u, 2);
    auto lsmc_call_50k = lsmc_50k.price(amer_call, params);
    auto lsmc_put_50k = lsmc_50k.price(amer_put, params);

    // Mid-tier configuration: 100k paths, deeper regression
    engines::MCAmericanLSMCEngine lsmc_100k(100000, 100, 4242u, 3);
    auto lsmc_call_100k = lsmc_100k.price(amer_call, params);
    auto lsmc_put_100k = lsmc_100k.price(amer_put, params);

    // High accuracy configuration: 200k paths, fine time grid
    engines::MCAmericanLSMCEngine lsmc_200k(200000, 200, 1337u, 3);
    auto lsmc_call_200k = lsmc_200k.price(amer_call, params);
    auto lsmc_put_200k = lsmc_200k.price(amer_put, params);

    std::cout << "Market Params: S=100, K=100, r=5%, q=0%, sigma=20%, T=1y\n\n";

    std::cout << "American Call (should match European price w/out dividend)\n";
    print_ref("Binomial Ref", binom_call);
    print_mc("LSMC (50k/50/L2)", lsmc_call_50k);
    print_mc("LSMC (100k/100/L3)", lsmc_call_100k);
    print_mc("LSMC (200k/200/L3)", lsmc_call_200k);

    std::cout << "\nAmerican Put (early exercise premium captured)\n";
    print_ref("Binomial Ref", binom_put);
    print_mc("LSMC (50k/50/L2)", lsmc_put_50k);
    print_mc("LSMC (100k/100/L3)", lsmc_put_100k);
    print_mc("LSMC (200k/200/L3)", lsmc_put_200k);
    std::cout << "  Early exercise premium (Binom): "
              << std::fixed << std::setprecision(6)
              << (binom_put.value - binom_put_euro.value) << '\n';

    return 0;
}

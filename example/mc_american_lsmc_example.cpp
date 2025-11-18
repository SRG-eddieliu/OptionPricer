#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BinomialCRR.hpp"
#include "../src/engines/MCAmericanLSMC.hpp"

namespace {

void print_mc(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(18) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev << "  StdErr: " << std::setw(10) << out.std_error
              << '\n';
}

void print_ref(const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Binomial Reference | Value: " << out.value << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{100.0, 100.0, 0.05, 0.00, 0.20, 1.0};
    core::OptionSpec amer_put{{params.K, core::OptionType::Put}, core::ExerciseStyle::American};

    engines::BinomialCRREngine binom_ref(4000, 0.0005);
    auto binom_price = binom_ref.price(amer_put, params);

    engines::MCAmericanLSMCEngine lsmc(75000, 75, 4242u, 3);
    auto lsmc_price = lsmc.price(amer_put, params);

    std::cout << "American Put via LSMC (Laguerre basis degree 3) vs Binomial reference\n";
    print_ref(binom_price);
    print_mc("LSMC (75k paths)", lsmc_price);

    return 0;
}

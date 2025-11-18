#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/MCEuropean.hpp"

namespace {

void print_mc(std::size_t paths, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "MC (" << std::setw(5) << paths << " paths) | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev << "  StdErr: " << std::setw(10) << out.std_error
              << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{120.0, 110.0, 0.02, 0.00, 0.15, 2.0};
    engines::BSEuropeanAnalytic bs;

    core::OptionSpec call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    auto analytic = bs.price(call, params);

    std::cout << "European Monte Carlo pricing (call) for S=120, K=110, r=2%, q=0%, sigma=15%, T=2\n";
    std::cout << "Black-Scholes Call baseline: " << std::fixed << std::setprecision(6) << analytic.value << "\n";

    for (std::size_t paths : {50000u, 75000u, 100000u}) {
        engines::MCEuropeanEngine engine(paths, 2024u + static_cast<unsigned int>(paths));
        auto results = engine.price(call, params);
        print_mc(paths, results);
    }

    return 0;
}

#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/MCEuropean.hpp"

namespace {

void print_mc(const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Value: " << std::setw(10) << out.value << "  StdDev: " << std::setw(10) << out.std_dev
              << "  StdErr: " << std::setw(10) << out.std_error << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{120.0, 110.0, 0.02, 0.00, 0.15, 2.0};
    engines::MCEuropeanEngine engine(50000, 2024u);

    core::OptionSpec call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    auto results = engine.price(call, params);

    std::cout << "European Monte Carlo pricing for S=120, K=110, r=2%, q=0%, sigma=15%, T=2 (50k paths)\n";
    print_mc(results);

    return 0;
}

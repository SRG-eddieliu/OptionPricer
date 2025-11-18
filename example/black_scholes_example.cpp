#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"

namespace {

void print_outputs(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(10) << label << " | Value: " << std::setw(10) << out.value
              << "  Delta: " << std::setw(10) << out.delta << "  Gamma: " << std::setw(10) << out.gamma
              << "  Vega: " << std::setw(10) << out.vega << '\n';
}

}  // namespace

int main() {
    core::OptionParams params{100.0, 105.0, 0.03, 0.01, 0.25, 0.5};
    engines::BSEuropeanAnalytic engine;

    core::OptionSpec call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    core::OptionSpec put{{params.K, core::OptionType::Put}, core::ExerciseStyle::European};

    auto call_outputs = engine.price(call, params);
    auto put_outputs = engine.price(put, params);

    std::cout << "Black-Scholes analytic pricing for S=100, K=105, r=3%, q=1%, sigma=25%, T=0.5\n\n";
    print_outputs("Call", call_outputs);
    print_outputs("Put", put_outputs);

    return 0;
}

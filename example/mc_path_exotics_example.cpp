#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/MCPathDependent.hpp"

namespace {

void print_result(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6)
              << std::setw(28) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev
              << "  StdErr: " << std::setw(10) << out.std_error << '\n';
}

}  // namespace

int main() {
    // Scenario A: 60k paths, 90 steps
    engines::MCPathDependentEngine engine_60k_90(60000, 90, 4321u);

    core::OptionParams asian_params{100.0, 95.0, 0.015, 0.00, 0.20, 1.0};
    core::PathDependentOptionSpec asian_spec{core::ExoticType::ArithmeticAsian, core::OptionType::Call, 95.0};
    auto asian_call = engine_60k_90.price(asian_spec, asian_params);

    core::OptionParams barrier_params{120.0, 115.0, 0.02, 0.00, 0.25, 0.75};
    core::PathDependentOptionSpec barrier_spec{core::ExoticType::Barrier, core::OptionType::Put, 115.0, 100.0,
                                               core::BarrierType::DownAndOut};
    auto barrier_put = engine_60k_90.price(barrier_spec, barrier_params);

    core::OptionParams lookback_params{90.0, 85.0, 0.01, 0.00, 0.30, 1.25};
    core::PathDependentOptionSpec lookback_spec{core::ExoticType::Lookback, core::OptionType::Call, 85.0};
    auto lookback_call = engine_60k_90.price(lookback_spec, lookback_params);

    // Scenario B: 120k paths, 180 steps (more accurate grid)
    engines::MCPathDependentEngine engine_120k_180(120000, 180, 9876u);
    auto asian_call_hi = engine_120k_180.price(asian_spec, asian_params);
    auto barrier_put_hi = engine_120k_180.price(barrier_spec, barrier_params);
    auto lookback_call_hi = engine_120k_180.price(lookback_spec, lookback_params);

    std::cout << "Path-Dependent Monte Carlo Examples\n";
    std::cout << "Scenario A: 60k paths, 90 steps\n";
    print_result("Arithmetic Asian Call", asian_call);
    print_result("Down-and-Out Put", barrier_put);
    print_result("Lookback Call", lookback_call);

    std::cout << "\nScenario B: 120k paths, 180 steps\n";
    print_result("Arithmetic Asian Call", asian_call_hi);
    print_result("Down-and-Out Put", barrier_put_hi);
    print_result("Lookback Call", lookback_call_hi);

    return 0;
}

#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/MCEuropean.hpp"

namespace {

void print_header(const std::string& title, const core::OptionParams& params) {
    std::cout << title << "\n";
    std::cout << "Params: S=" << params.S << ", K=" << params.K
              << ", r=" << params.r << ", q=" << params.q
              << ", sigma=" << params.sig << ", T=" << params.T << "\n";
}

void print_mc(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6)
              << std::setw(28) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev
              << "  StdErr: " << std::setw(10) << out.std_error << '\n';
}

engines::PriceOutputs run_mc(std::size_t paths,
                             std::uint64_t seed,
                             engines::BaseMCEngine::VarianceReductionMethod vr,
                             const core::OptionSpec& spec,
                             const core::OptionParams& params) {
    engines::MCEuropeanEngine engine(paths, 1, seed, vr);
    return engine.price(spec, params);
}

}  // namespace

int main() {
    core::OptionParams params{100.0, 100.0, 0.01, 0.00, 0.25, 1.0};
    engines::BSEuropeanAnalytic bs;

    core::OptionSpec call{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    auto analytic = bs.price(call, params);

    print_header("European Call Monte Carlo — Variance Reduction Comparison", params);
    std::cout << "Black-Scholes Call baseline: " << std::fixed << std::setprecision(6) << analytic.value << "\n\n";

    for (std::size_t paths : {25000u, 50000u, 100000u}) {
        auto vanilla = run_mc(paths, 2024u + paths, engines::VarianceReductionMethod::None, call, params);
        auto antithetic =
            run_mc(paths, 1337u + paths, engines::VarianceReductionMethod::AntitheticVariates, call, params);

        std::cout << "-- Paths: " << paths << " --\n";
        print_mc("Plain MC", vanilla);
        print_mc("MC + Antithetic", antithetic);
        std::cout << '\n';
    }

    return 0;
}

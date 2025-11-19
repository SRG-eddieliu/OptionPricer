#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/BinomialCRR.hpp"
#include "../src/engines/MCAmericanLSMC.hpp"
#include "../src/engines/MCEuropean.hpp"

namespace {

using VR = engines::BaseMCEngine::VarianceReductionMethod;

engines::PriceOutputs run_euro(std::size_t paths,
                               std::uint64_t seed,
                               VR method,
                               const core::OptionSpec& spec,
                               const core::OptionParams& params) {
    engines::MCEuropeanEngine engine(paths, 1, seed,
                                     method == VR::AntitheticMomentMatching ? VR::AntitheticVariates : method);
    return engine.price(spec, params);
}

engines::PriceOutputs run_amer(std::size_t paths,
                               std::size_t steps,
                               std::uint64_t seed,
                               VR method,
                               const core::OptionSpec& spec,
                               const core::OptionParams& params) {
    engines::MCAmericanLSMCEngine engine(paths, steps, seed, 2,
                                         method == VR::AntitheticMomentMatching ? VR::AntitheticVariates : method);
    return engine.price(spec, params);
}

void print_mc(const std::string& label, const engines::PriceOutputs& out) {
    std::cout << std::fixed << std::setprecision(6)
              << std::setw(32) << label << " | Value: " << std::setw(10) << out.value
              << "  StdDev: " << std::setw(10) << out.std_dev
              << "  StdErr: " << std::setw(10) << out.std_error << '\n';
}

}  // namespace

int main() {
    // European call setup
    core::OptionParams euro_params{120.0, 110.0, 0.02, 0.00, 0.20, 1.0};
    core::OptionSpec euro_call{{euro_params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    engines::BSEuropeanAnalytic bs;
    auto bs_call = bs.price(euro_call, euro_params);

    std::cout << "European Call Monte Carlo Variance Strategies\n";
    std::cout << "Params: S=" << euro_params.S << ", K=" << euro_params.K << ", r=" << euro_params.r
              << ", q=" << euro_params.q << ", sigma=" << euro_params.sig << ", T=" << euro_params.T << "\n";
    std::cout << "Black-Scholes baseline: " << std::fixed << std::setprecision(6) << bs_call.value << "\n\n";

    for (std::size_t paths : {30000u, 60000u, 90000u}) {
        std::cout << "-- Paths: " << paths << " --\n";
        print_mc("Plain MC", run_euro(paths, 8000u + paths, VR::None, euro_call, euro_params));
        print_mc("MC + Antithetic",
                 run_euro(paths, 8100u + paths, VR::AntitheticVariates, euro_call, euro_params));
        print_mc("MC + Moment Matching",
                 run_euro(paths, 8200u + paths, VR::MomentMatching, euro_call, euro_params));
        print_mc("MC + Antithetic+Moment",
                 run_euro(paths, 8300u + paths, VR::AntitheticMomentMatching, euro_call, euro_params));
        std::cout << '\n';
    }

    // American put setup
    core::OptionParams amer_params{100.0, 100.0, 0.04, 0.00, 0.25, 1.0};
    core::OptionSpec amer_put{{amer_params.K, core::OptionType::Put}, core::ExerciseStyle::American};
    engines::BinomialCRREngine binom_ref(4000, 0.0005);
    auto binom_put = binom_ref.price(amer_put, amer_params);

    std::cout << "American Put via LSMC (variance strategies)\n";
    std::cout << "Params: S=" << amer_params.S << ", K=" << amer_params.K << ", r=" << amer_params.r
              << ", q=" << amer_params.q << ", sigma=" << amer_params.sig << ", T=" << amer_params.T << "\n";
    std::cout << "Binomial baseline: " << std::fixed << std::setprecision(6) << binom_put.value << "\n\n";

    for (std::size_t paths : {50000u, 100000u, 150000u}) {
        std::cout << "-- Paths: " << paths << " --\n";
        print_mc("Plain MC", run_amer(paths, 75, 8400u + paths, VR::None, amer_put, amer_params));
        print_mc("MC + Antithetic",
                 run_amer(paths, 75, 8500u + paths, VR::AntitheticVariates, amer_put, amer_params));
        print_mc("MC + Moment Matching",
                 run_amer(paths, 75, 8600u + paths, VR::MomentMatching, amer_put, amer_params));
        print_mc("MC + Antithetic+Moment",
                 run_amer(paths, 75, 8700u + paths, VR::AntitheticMomentMatching, amer_put, amer_params));
        std::cout << '\n';
    }

    return 0;
}

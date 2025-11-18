#include "engines/MCEuropean.hpp"

#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

#include "math/Stats.hpp"

namespace engines {

PriceOutputs MCEuropeanEngine::price(const core::OptionSpec& spec,
                                     const core::OptionParams& params) const {
    // European options only
    if (spec.exercise != core::ExerciseStyle::European) {
        throw std::invalid_argument("MCEuropeanEngine: European exercise style required");
    }

    // Handle edge cases (zero time or zero volatility)
    if (params.T <= 0.0 || params.sig <= 0.0) {
        PriceOutputs outputs{};
        outputs.value = spec.payoff(params.S);
        return outputs;
    }

    auto paths = generatePaths(params);
    std::vector<double> discounted_payoffs;
    discounted_payoffs.reserve(paths.size());

    double discount = std::exp(-params.r * params.T);
    for (const auto& path : paths) {
        double payoff = spec.payoff(path.back());
        discounted_payoffs.push_back(discount * payoff);
    }

    // Apply variance reduction if configured (to be implemented by subclasses or strategies)
    applyVarianceReduction(discounted_payoffs, spec, params);

    // Compute statistics
    PriceOutputs outputs{};
    outputs.value = math::stats::mean(discounted_payoffs);
    outputs.std_dev = math::stats::standard_deviation(discounted_payoffs);
    outputs.std_error = math::stats::standard_error(discounted_payoffs);

    return outputs;
}

}  // namespace engines

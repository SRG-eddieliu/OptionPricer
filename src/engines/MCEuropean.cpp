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

    // Initialize RNG
    std::mt19937_64 rng(seed_);
    std::normal_distribution<double> standard_normal(0.0, 1.0);
    std::vector<double> discounted_payoffs;
    discounted_payoffs.reserve(paths_);

    // Pre-compute constants
    double drift = (params.r - params.q - 0.5 * params.sig * params.sig) * params.T;
    double diffusion = params.sig * std::sqrt(params.T);
    double discount = std::exp(-params.r * params.T);

    // Simulate paths and compute payoffs
    for (std::size_t i = 0; i < paths_; ++i) {
        double z = standard_normal(rng);
        double ST = params.S * std::exp(drift + diffusion * z);
        double payoff = spec.payoff(ST);
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

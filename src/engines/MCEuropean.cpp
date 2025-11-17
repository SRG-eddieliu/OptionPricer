#include "engines/MCEuropean.hpp"

#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

#include "math/Stats.hpp"

namespace engines {

PriceOutputs MCEuropeanEngine::price(const core::OptionSpec& spec,
                                     const core::OptionParams& params) const {
    if (spec.exercise != core::ExerciseStyle::European) {
        throw std::invalid_argument("Monte Carlo engine currently supports European exercise only");
    }
    if (params.T <= 0.0 || params.sig <= 0.0) {
        PriceOutputs outputs{};
        outputs.value = spec.payoff(params.S);
        return outputs;
    }

    std::mt19937_64 rng(seed_);
    std::normal_distribution<double> standard_normal(0.0, 1.0);
    std::vector<double> discounted_payoffs;
    discounted_payoffs.reserve(paths_);

    double drift = (params.r - params.q - 0.5 * params.sig * params.sig) * params.T;
    double diffusion = params.sig * std::sqrt(params.T);
    double discount = std::exp(-params.r * params.T);

    for (std::size_t i = 0; i < paths_; ++i) {
        double z = standard_normal(rng);
        double ST = params.S * std::exp(drift + diffusion * z);
        double payoff = spec.payoff(ST);
        discounted_payoffs.push_back(discount * payoff);
    }

    PriceOutputs outputs{};
    outputs.value = math::stats::mean(discounted_payoffs);
    outputs.std_dev = math::stats::standard_deviation(discounted_payoffs);
    outputs.std_error = math::stats::standard_error(discounted_payoffs);
    return outputs;
}

} // namespace engines

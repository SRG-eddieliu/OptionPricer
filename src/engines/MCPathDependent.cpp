#include "engines/MCPathDependent.hpp"

#include <cmath>
#include <stdexcept>

#include "math/Stats.hpp"

namespace engines {
namespace {

bool barrier_hit(const std::vector<double>& path, double barrier, core::BarrierType type) {
    switch (type) {
        case core::BarrierType::UpAndOut:
        case core::BarrierType::UpAndIn:
            for (double spot : path) {
                if (spot >= barrier) {
                    return true;
                }
            }
            return false;
        case core::BarrierType::DownAndOut:
        case core::BarrierType::DownAndIn:
            for (double spot : path) {
                if (spot <= barrier) {
                    return true;
                }
            }
            return false;
    }
    return false;
}

}  // namespace

PriceOutputs MCPathDependentEngine::price(const core::PathDependentOptionSpec& spec,
                                          const core::OptionParams& params) const {
    auto paths = generatePaths(params);
    std::vector<double> discounted;
    discounted.reserve(paths.size());

    double discount = std::exp(-params.r * params.T);

    for (const auto& path : paths) {
        double payoff = 0.0;
        switch (spec.type) {
            case core::ExoticType::ArithmeticAsian:
                payoff = asian_payoff(spec, path);
                break;
            case core::ExoticType::Barrier:
                payoff = barrier_payoff(spec, path);
                break;
            case core::ExoticType::Lookback:
                payoff = lookback_payoff(spec, path);
                break;
        }
        discounted.push_back(discount * payoff);
    }

    core::OptionSpec dummy_spec{};
    applyVarianceReduction(discounted, dummy_spec, params);

    PriceOutputs outputs{};
    outputs.value = math::stats::mean(discounted);
    outputs.std_dev = math::stats::standard_deviation(discounted);
    outputs.std_error = math::stats::standard_error(discounted);
    return outputs;
}

PriceOutputs MCPathDependentEngine::price(const core::OptionSpec& spec,
                                          const core::OptionParams& params) const {
    (void)spec;
    (void)params;
    throw std::invalid_argument("MCPathDependentEngine requires PathDependentOptionSpec");
}

double MCPathDependentEngine::asian_payoff(const core::PathDependentOptionSpec& spec,
                                            const std::vector<double>& path) {
    double sum = 0.0;
    for (double spot : path) {
        sum += spot;
    }
    double avg = sum / static_cast<double>(path.size());
    double intrinsic = (spec.option_type == core::OptionType::Call) ? (avg - spec.strike) : (spec.strike - avg);
    return std::max(intrinsic, 0.0);
}

double MCPathDependentEngine::barrier_payoff(const core::PathDependentOptionSpec& spec,
                                              const std::vector<double>& path) {
    bool hit = barrier_hit(path, spec.barrier_level, spec.barrier_type);
    bool knock_in = (spec.barrier_type == core::BarrierType::UpAndIn ||
                     spec.barrier_type == core::BarrierType::DownAndIn);
    if ((knock_in && !hit) || (!knock_in && hit)) {
        return 0.0;
    }
    double ST = path.back();
    double intrinsic = (spec.option_type == core::OptionType::Call) ? (ST - spec.strike) : (spec.strike - ST);
    return std::max(intrinsic, 0.0);
}

double MCPathDependentEngine::lookback_payoff(const core::PathDependentOptionSpec& spec,
                                               const std::vector<double>& path) {
    double max_spot = path.front();
    double min_spot = path.front();
    for (double spot : path) {
        max_spot = std::max(max_spot, spot);
        min_spot = std::min(min_spot, spot);
    }
    if (spec.option_type == core::OptionType::Call) {
        return std::max(max_spot - spec.strike, 0.0);
    }
    return std::max(spec.strike - min_spot, 0.0);
}

}  // namespace engines

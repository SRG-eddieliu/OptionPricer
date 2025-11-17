#include "engines/TrinomialTree.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace engines {

namespace {
constexpr double SQRT3 = 1.7320508075688772;
}

double TrinomialTreeEngine::value_from_tree(const core::OptionSpec& spec,
                                            const core::OptionParams& params,
                                            double spot) const {
    if (steps_ == 0 || params.T <= 0.0 || params.sig <= 0.0) {
        return spec.payoff(spot);
    }

    double dt = params.T / static_cast<double>(steps_);
    double sqrt_dt = std::sqrt(dt);
    double disc = std::exp(-params.r * dt);
    double u = std::exp(params.sig * std::sqrt(3.0 * dt));
    double d = 1.0 / u;

    double drift = params.r - params.q;
    double a = drift - 0.5 * params.sig * params.sig;
    double pu = 1.0 / 6.0 + (a * sqrt_dt) / (2.0 * params.sig * SQRT3);
    double pd = 1.0 / 6.0 - (a * sqrt_dt) / (2.0 * params.sig * SQRT3);
    double pm = 1.0 - pu - pd;

    pu = std::max(0.0, pu);
    pd = std::max(0.0, pd);
    pm = std::max(0.0, pm);
    double sum = pu + pm + pd;
    if (sum == 0.0) {
        pu = pd = 0.25;
        pm = 0.5;
    } else {
        pu /= sum;
        pm /= sum;
        pd /= sum;
    }

    int size = static_cast<int>(2 * steps_ + 1);
    int offset = static_cast<int>(steps_);
    std::vector<double> option_values(size, 0.0);
    std::vector<double> next_values(size, 0.0);

    for (int j = -offset; j <= offset; ++j) {
        double ST = spot * std::pow(u, static_cast<double>(j));
        option_values[j + offset] = spec.payoff(ST);
    }

    for (std::size_t step = steps_; step > 0; --step) {
        std::fill(next_values.begin(), next_values.end(), 0.0);
        int limit = static_cast<int>(step) - 1;
        for (int j = -limit; j <= limit; ++j) {
            double continuation = disc * (pu * option_values[(j + 1) + offset] +
                                          pm * option_values[j + offset] +
                                          pd * option_values[(j - 1) + offset]);
            double node_spot = spot * std::pow(u, static_cast<double>(j));
            if (spec.exercise == core::ExerciseStyle::American) {
                next_values[j + offset] =
                    std::max(continuation, spec.payoff(node_spot));
            } else {
                next_values[j + offset] = continuation;
            }
        }
        option_values.swap(next_values);
    }

    return option_values[offset];
}

PriceOutputs TrinomialTreeEngine::price(const core::OptionSpec& spec,
                                        const core::OptionParams& params) const {
    if (steps_ == 0) {
        throw std::invalid_argument("Trinomial engine requires at least one step");
    }

    PriceOutputs outputs{};
    double base = value_from_tree(spec, params, params.S);
    outputs.value = base;

    if (params.S > 0.0 && bump_size_ > 0.0) {
        double log_bump = bump_size_;
        double spot_up = params.S * std::exp(log_bump);
        double spot_down = params.S * std::exp(-log_bump);
        double up = value_from_tree(spec, params, spot_up);
        double down = value_from_tree(spec, params, spot_down);
        double h_up = spot_up - params.S;
        double h_down = params.S - spot_down;
        double denom_delta = spot_up - spot_down;
        if (denom_delta > 0.0) {
            outputs.delta = (up - down) / denom_delta;
        }
        double gamma_denom = h_up * h_down * (h_up + h_down);
        if (h_up > 0.0 && h_down > 0.0 && gamma_denom != 0.0) {
            outputs.gamma = 2.0 *
                            (h_down * up - (h_up + h_down) * base + h_up * down) /
                            gamma_denom;
        }
    }

    outputs.std_dev = 0.0;
    outputs.std_error = 0.0;
    return outputs;
}

PriceOutputs TrinomialTreeEngine::priceAmerican(const core::OptionSpec& spec,
                                                const core::OptionParams& params) const {
    core::OptionSpec american_spec = spec;
    american_spec.exercise = core::ExerciseStyle::American;
    return price(american_spec, params);
}

} // namespace engines

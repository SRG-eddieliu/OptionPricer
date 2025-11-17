#include "engines/BinomialCRR.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace engines {

double BinomialCRREngine::value_from_tree(const core::OptionSpec& spec,
                                          const core::OptionParams& params,
                                          double spot) const {
    if (steps_ == 0 || params.T <= 0.0) {
        return spec.payoff(spot);
    }

    double dt = params.T / static_cast<double>(steps_);
    double u = std::exp(params.sig * std::sqrt(dt));
    double d = 1.0 / u;
    double disc = std::exp(-params.r * dt);
    double drift = std::exp((params.r - params.q) * dt);
    double p = (drift - d) / (u - d);
    p = std::clamp(p, 0.0, 1.0);

    std::vector<double> option_values(steps_ + 1);

    double ST = spot * std::pow(d, static_cast<double>(steps_));
    double up_over_down = u / d;
    for (std::size_t i = 0; i <= steps_; ++i) {
        option_values[i] = spec.payoff(ST);
        ST *= up_over_down;
    }

    for (std::size_t step = steps_; step-- > 0;) {
        for (std::size_t i = 0; i <= step; ++i) {
            double continuation = disc * (p * option_values[i + 1] +
                                          (1.0 - p) * option_values[i]);
            if (spec.exercise == core::ExerciseStyle::American) {
                double node_spot = spot * std::pow(u, static_cast<double>(i)) *
                                   std::pow(d, static_cast<double>(step - i));
                double exercise = spec.payoff(node_spot);
                option_values[i] = std::max(continuation, exercise);
            } else {
                option_values[i] = continuation;
            }
        }
    }

    return option_values[0];
}

PriceOutputs BinomialCRREngine::price(const core::OptionSpec& spec,
                                      const core::OptionParams& params) const {
    if (spec.exercise == core::ExerciseStyle::American) {
        return priceAmerican(spec, params);
    }
    return priceEuropean(spec, params);
}

PriceOutputs BinomialCRREngine::priceEuropean(const core::OptionSpec& spec,
                                              const core::OptionParams& params) const {
    if (steps_ == 0) {
        throw std::invalid_argument("Binomial engine requires at least one step");
    }

    // Ensure we price with European exercise logic
    core::OptionSpec euro_spec = spec;
    euro_spec.exercise = core::ExerciseStyle::European;

    PriceOutputs outputs{};
    double base = value_from_tree(euro_spec, params, params.S);
    outputs.value = base;

    if (params.S > 0.0 && bump_size_ > 0.0) {
        double log_bump = bump_size_;
        double spot_up = params.S * std::exp(log_bump);
        double spot_down = params.S * std::exp(-log_bump);
        double up = value_from_tree(euro_spec, params, spot_up);
        double down = value_from_tree(euro_spec, params, spot_down);
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

PriceOutputs BinomialCRREngine::priceAmerican(const core::OptionSpec& spec,
                                              const core::OptionParams& params) const {
    // Ensure we price with American exercise logic
    core::OptionSpec american_spec = spec;
    american_spec.exercise = core::ExerciseStyle::American;

    if (steps_ == 0) {
        throw std::invalid_argument("Binomial engine requires at least one step");
    }

    PriceOutputs outputs{};
    double base = value_from_tree(american_spec, params, params.S);
    outputs.value = base;

    if (params.S > 0.0 && bump_size_ > 0.0) {
        double log_bump = bump_size_;
        double spot_up = params.S * std::exp(log_bump);
        double spot_down = params.S * std::exp(-log_bump);
        double up = value_from_tree(american_spec, params, spot_up);
        double down = value_from_tree(american_spec, params, spot_down);
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

} // namespace engines

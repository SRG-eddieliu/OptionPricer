#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "core/Types.hpp"

namespace core {

inline void validate_market(const OptionParams& p) {
    for (double value : {p.S, p.K, p.r, p.q, p.sig, p.T}) {
        if (!std::isfinite(value)) throw std::invalid_argument("Market inputs must be finite");
    }
    if (p.S <= 0 || p.K <= 0 || p.sig < 0 || p.T < 0) {
        throw std::invalid_argument("Require positive spot/strike and nonnegative volatility/maturity");
    }
}

inline void validate_strike(double strike, const OptionParams& p) {
    validate_market(p);
    if (!std::isfinite(strike) || strike <= 0 ||
        std::abs(strike-p.K) > 1e-12*std::max(strike,p.K)) {
        throw std::invalid_argument("Payoff strike must match OptionParams.K");
    }
}

inline double deterministic_value(const OptionSpec& spec, const OptionParams& p,
                                  double spot, std::size_t steps = 1) {
    auto value_at = [&](double t) {
        return std::exp(-p.r*t)*spec.payoff(spot*std::exp((p.r-p.q)*t));
    };
    double value = value_at(p.T);
    if (spec.exercise == ExerciseStyle::American) {
        // Exercise is restricted to the same time grid as the numerical engine.
        for (std::size_t i=0; i<steps; ++i) {
            value = std::max(value,value_at(p.T*static_cast<double>(i)/steps));
        }
    }
    return value;
}
} // namespace core

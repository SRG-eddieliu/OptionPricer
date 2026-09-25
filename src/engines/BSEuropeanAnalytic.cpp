#include "engines/BSEuropeanAnalytic.hpp"

#include <cmath>
#include <stdexcept>
#include <limits>

#include "math/Normal.hpp"
#include "core/Validation.hpp"

namespace engines {

PriceOutputs BSEuropeanAnalytic::price(const core::OptionSpec& spec,
                                       const core::OptionParams& params) const {
    if (spec.exercise != core::ExerciseStyle::European) {
        throw std::invalid_argument("Black-Scholes engine requires European exercise");
    }
    core::validate_strike(spec.payoff.strike, params);

    PriceOutputs outputs{};

    if (params.T <= 0.0 || params.sig <= 0.0) {
        outputs.value = core::deterministic_value(spec, params, params.S);
        outputs.delta = outputs.gamma = outputs.vega = outputs.theta = outputs.rho =
            std::numeric_limits<double>::quiet_NaN();
        return outputs;
    }

    double sqrtT = std::sqrt(params.T);
    double d1 = (std::log(params.S / params.K) +
                 (params.r - params.q + 0.5 * params.sig * params.sig) * params.T) /
                (params.sig * sqrtT);
    double d2 = d1 - params.sig * sqrtT;

    double disc_r = std::exp(-params.r * params.T);
    double disc_q = std::exp(-params.q * params.T);

    if (spec.payoff.type == core::OptionType::Call) {
        outputs.value = params.S * disc_q * math::normal::N(d1) -
                        params.K * disc_r * math::normal::N(d2);
        outputs.delta = disc_q * math::normal::N(d1);
        outputs.theta = -(params.S * disc_q * math::normal::n(d1) * params.sig) /
                            (2.0 * sqrtT) -
                        params.r * params.K * disc_r * math::normal::N(d2) +
                        params.q * params.S * disc_q * math::normal::N(d1);
        outputs.rho = params.K * params.T * disc_r * math::normal::N(d2);
    } else {
        outputs.value = params.K * disc_r * math::normal::N(-d2) -
                        params.S * disc_q * math::normal::N(-d1);
        outputs.delta = disc_q * (math::normal::N(d1) - 1.0);
        outputs.theta = -(params.S * disc_q * math::normal::n(d1) * params.sig) /
                            (2.0 * sqrtT) +
                        params.r * params.K * disc_r * math::normal::N(-d2) -
                        params.q * params.S * disc_q * math::normal::N(-d1);
        outputs.rho = -params.K * params.T * disc_r * math::normal::N(-d2);
    }

    outputs.gamma = (disc_q * math::normal::n(d1)) /
                    (params.S * params.sig * sqrtT);
    outputs.vega = params.S * disc_q * math::normal::n(d1) * sqrtT;
    outputs.std_dev = 0.0;
    outputs.std_error = 0.0;
    return outputs;
}

} // namespace engines

#pragma once

#include "core/Types.hpp"

namespace engines {

struct PriceOutputs {
    double value{0.0};
    double delta{0.0};
    double gamma{0.0};
    double vega{0.0};
    double theta{0.0};
    double rho{0.0};
    double std_dev{0.0};
    double std_error{0.0};
};

class PricingEngine {
  public:
    virtual ~PricingEngine() = default;
    virtual PriceOutputs price(const core::OptionSpec& spec, const core::OptionParams& params) const = 0;
};

} // namespace engines

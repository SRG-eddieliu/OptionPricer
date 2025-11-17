#pragma once

#include "engines/PricingEngine.hpp"

namespace engines {

class BSEuropeanAnalytic : public PricingEngine {
  public:
    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;
};

} // namespace engines

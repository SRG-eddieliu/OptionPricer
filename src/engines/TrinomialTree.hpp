#pragma once

#include <cstddef>

#include "engines/PricingEngine.hpp"

namespace engines {

class TrinomialTreeEngine : public PricingEngine {
  public:
    explicit TrinomialTreeEngine(std::size_t steps = 4000, double bump = 0.0005)
        : steps_(steps), bump_size_(bump) {}

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;
    PriceOutputs priceAmerican(const core::OptionSpec& spec,
                               const core::OptionParams& params) const;

  private:
    double value_from_tree(const core::OptionSpec& spec, const core::OptionParams& params,
                           double spot) const;

    std::size_t steps_;
    double bump_size_;
};

} // namespace engines

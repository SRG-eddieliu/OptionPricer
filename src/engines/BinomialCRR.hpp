#pragma once

#include <cstddef>

#include "engines/PricingEngine.hpp"

namespace engines {

class BinomialCRREngine : public PricingEngine {
  private:
    double value_from_tree(const core::OptionSpec& spec, const core::OptionParams& params,
                           double spot) const;

    std::size_t steps_;
    double bump_size_;

  public:
    // Constructor with number of steps and bump size for Greeks
    explicit BinomialCRREngine(std::size_t steps = 4000, double bump = 0.0005)
        : steps_(steps), bump_size_(bump) {}

    PriceOutputs price(const core::OptionSpec& spec,
               const core::OptionParams& params) const override;

    private:
    // Separate implementations for clarity; public `price()` dispatches
    PriceOutputs priceEuropean(const core::OptionSpec& spec,
                   const core::OptionParams& params) const;
    PriceOutputs priceAmerican(const core::OptionSpec& spec,
                   const core::OptionParams& params) const;

};

} // namespace engines

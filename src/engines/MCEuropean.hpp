#pragma once

#include <cstddef>

#include "engines/PricingEngine.hpp"

namespace engines {

class MCEuropeanEngine : public PricingEngine {

    private:
    std::size_t paths_;
    unsigned int seed_;
    
  public:
    explicit MCEuropeanEngine(std::size_t paths = 20000, unsigned int seed = 5489u)
        : paths_(paths), seed_(seed) {}

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;


};

} // namespace engines

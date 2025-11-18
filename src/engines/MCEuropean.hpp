#pragma once

#include <cstddef>

#include "engines/MCEngine.hpp"

namespace engines {

// Monte Carlo engine dedicated to European vanilla options.
class MCEuropeanEngine : public MCEngine {

   public:
    explicit MCEuropeanEngine(std::size_t paths = 20000,
                              unsigned int seed = 5489u,
                              VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : MCEngine(paths, seed, vr_method) {}

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;

};  // class MCEuropeanEngine

}  // namespace engines

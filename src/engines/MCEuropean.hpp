#pragma once

#include <cstddef>

#include "engines/MCEngine.hpp"

namespace engines {

// Monte Carlo engine dedicated to European vanilla options.
class MCEuropeanEngine : public BaseMCEngine {

   public:
    explicit MCEuropeanEngine(std::size_t paths = 20000,
                              std::size_t time_steps = 1,
                              std::uint64_t seed = 5489u,
                              VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : BaseMCEngine(paths, time_steps, seed, vr_method) {}

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;

};  // class MCEuropeanEngine

}  // namespace engines

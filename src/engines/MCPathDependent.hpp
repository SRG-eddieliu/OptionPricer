#pragma once

#include "core/Types.hpp"
#include "engines/MCEngine.hpp"

namespace engines {

class MCPathDependentEngine : public BaseMCEngine {
   public:
    explicit MCPathDependentEngine(std::size_t paths = 50000,
                                   std::size_t time_steps = 75,
                                   std::uint64_t seed = 5489u,
                                   VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : BaseMCEngine(paths, time_steps, seed, vr_method) {}

    PriceOutputs price(const core::PathDependentOptionSpec& spec,
                       const core::OptionParams& params) const;

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;

   private:
    static double asian_payoff(const core::PathDependentOptionSpec& spec,
                               const std::vector<double>& path);
    static double barrier_payoff(const core::PathDependentOptionSpec& spec,
                                 const std::vector<double>& path);
    static double lookback_payoff(const core::PathDependentOptionSpec& spec,
                                  const std::vector<double>& path);
};

}  // namespace engines

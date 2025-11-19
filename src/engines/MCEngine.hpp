#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "engines/PricingEngine.hpp"

namespace engines {

class BaseMCEngine : public PricingEngine {
   public:
    enum class VarianceReductionMethod {
        None,
        AntitheticVariates,
        MomentMatching,
        AntitheticMomentMatching,
        QuasiMonteCarlo,
        Multilevel
    };

    explicit BaseMCEngine(std::size_t paths = 20000,
                          std::size_t time_steps = 1,
                          std::uint64_t seed = 5489u,
                          VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : paths_(paths),
          time_steps_(time_steps > 0 ? time_steps : 1),
          seed_(seed),
          vr_method_(vr_method) {}

    virtual ~BaseMCEngine() = default;

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override = 0;

   protected:
    std::vector<std::vector<double>> generatePaths(const core::OptionParams& params) const;

    virtual void applyVarianceReduction(std::vector<double>& discounted_payoffs,
                                        const core::OptionSpec& spec,
                                        const core::OptionParams& params) const;

    void setVarianceReduction(VarianceReductionMethod method) { vr_method_ = method; }
    VarianceReductionMethod getVarianceReduction() const { return vr_method_; }
    std::size_t getTimeSteps() const { return time_steps_; }

    std::size_t paths_;
    std::size_t time_steps_;
    std::uint64_t seed_;
    VarianceReductionMethod vr_method_ = VarianceReductionMethod::None;
};

using VarianceReductionMethod = BaseMCEngine::VarianceReductionMethod;

}  // namespace engines

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "engines/PricingEngine.hpp"

namespace engines {

// Base class for Monte Carlo pricing engines (European, American LSMC, exotic, etc.).
class MCEngine : public PricingEngine {
    
   public:
    enum class VarianceReductionMethod {
        None,
        ControlVariate,
        AntitheticVariates,
        QuasiMonteCarlo,
        Multilevel
    };

    explicit MCEngine(std::size_t paths = 20000,
                      unsigned int seed = 5489u,
                      VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : paths_(paths), seed_(seed), vr_method_(vr_method) {}

    virtual ~MCEngine() = default;

    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override = 0;

    virtual void applyVarianceReduction(std::vector<double>& discounted_payoffs,
                                        const core::OptionSpec& spec,
                                        const core::OptionParams& params) const {
        (void)discounted_payoffs;
        (void)spec;
        (void)params;
    }

    void setVarianceReduction(VarianceReductionMethod method) { vr_method_ = method; }
    VarianceReductionMethod getVarianceReduction() const { return vr_method_; }

   protected:
    std::size_t paths_;
    unsigned int seed_;
    VarianceReductionMethod vr_method_ = VarianceReductionMethod::None;

};  // class MCEngine

}  // namespace engines

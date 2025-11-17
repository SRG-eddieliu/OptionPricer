#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "engines/PricingEngine.hpp"

namespace engines {

/**
 * @class MCEngine
 * @brief Abstract base class for Monte Carlo option pricing implementations.
 *
 * Provides common framework for all Monte Carlo variants (European, American LSMC, Exotic).
 * Supports variance reduction techniques that can be applied across all MC methods.
 *
 * Subclasses must implement:
 * - price(): Core MC simulation and payoff evaluation
 * - applyVarianceReduction(): Optional variance reduction techniques
 */
class MCEngine : public PricingEngine {
   protected:
    std::size_t paths_;
    unsigned int seed_;

    /**
     * @enum VarianceReductionMethod
     * @brief Variance reduction techniques applicable to all MC variants.
     */
    enum class VarianceReductionMethod {
        None,               ///< No variance reduction
        ControlVariate,     ///< Use analytical price as control
        AntitheticVariates, ///< Generate paired paths with opposite normals
        QuasiMonteCarlo,    ///< Use low-discrepancy sequences (Sobol/Halton)
        Multilevel          ///< Multilevel Monte Carlo
    };

    VarianceReductionMethod vr_method_ = VarianceReductionMethod::None;

   public:
    /**
     * Constructor for MCEngine.
     * @param paths Number of Monte Carlo paths to simulate
     * @param seed Random number generator seed
     * @param vr_method Variance reduction technique to apply
     */
    explicit MCEngine(std::size_t paths = 20000,
                      unsigned int seed = 5489u,
                      VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : paths_(paths), seed_(seed), vr_method_(vr_method) {}

    virtual ~MCEngine() = default;

    /**
     * @brief Price an option using Monte Carlo simulation.
     * Must be implemented by concrete MC variants (European, American LSMC, etc.)
     */
    virtual PriceOutputs price(const core::OptionSpec& spec,
                               const core::OptionParams& params) const override = 0;

    /**
     * @brief Apply variance reduction to simulated paths.
     * Can be overridden by subclasses. Default implementation does nothing.
     */
    virtual void applyVarianceReduction(std::vector<double>& discounted_payoffs,
                                        const core::OptionSpec& spec,
                                        const core::OptionParams& params) const {
        // Default: no variance reduction applied
        // Subclasses can override to implement specific VR techniques
    }

    // Setters for variance reduction configuration
    void setVarianceReduction(VarianceReductionMethod method) { vr_method_ = method; }

    VarianceReductionMethod getVarianceReduction() const { return vr_method_; }

};  // class MCEngine

}  // namespace engines

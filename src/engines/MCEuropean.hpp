#pragma once

#include <cstddef>

#include "engines/MCEngine.hpp"

namespace engines {

/**
 * @class MCEuropeanEngine
 * @brief Monte Carlo pricing for European vanilla options.
 *
 * Simulates stock price paths under risk-neutral measure and computes payoffs at maturity.
 * Inherits variance reduction capabilities from MCEngine base class.
 */
class MCEuropeanEngine : public MCEngine {

   public:
    /**
     * Constructor for European MC engine.
     * @param paths Number of Monte Carlo paths (default: 20000)
     * @param seed RNG seed (default: 5489u)
     * @param vr_method Variance reduction technique (default: None)
     */
    explicit MCEuropeanEngine(std::size_t paths = 20000,
                              unsigned int seed = 5489u,
                              VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : MCEngine(paths, seed, vr_method) {}

    /**
     * @brief Price a European option using Monte Carlo simulation.
     * @param spec Option specification (exercise style, payoff)
     * @param params Market parameters
     * @return PriceOutputs with value, std_dev, and std_error
     * @throws std::invalid_argument if exercise style is not European
     */
    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;

};  // class MCEuropeanEngine

}  // namespace engines

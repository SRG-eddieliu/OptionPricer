#pragma once

#include <cstddef>
#include <vector>

#include "engines/MCEngine.hpp"

namespace engines {

/**
 * @class MCAmericanLSMCEngine
 * @brief Least-Squares Monte Carlo (LSMC) for American vanilla options.
 *
 * Implements the Longstaff-Schwartz algorithm for pricing American options via regression
 * on pathwise continuation values.
 *
 * Algorithm Overview:
 * 1. Simulate stock price paths from t=0 to t=T
 * 2. At each time step (backwards from T to 0):
 *    - Identify in-the-money (ITM) paths
 *    - Regress continuation value on basis functions of spot price
 *    - Compare intrinsic value vs. continuation value
 *    - Exercise early if intrinsic > continuation
 * 3. Discount all cash flows back to present
 *
 * Variance reduction techniques inherited from MCEngine base class can be applied.
 *
 * Reference: Longstaff & Schwartz (2001), "Valuing American Options by Simulation"
 */
class MCAmericanLSMCEngine : public MCEngine {

   private:
    std::size_t time_steps_;  ///< Number of time discretization steps
    int polynomial_degree_;   ///< Degree of Laguerre polynomial basis (default: 2)

   public:
    /**
     * Constructor for American LSMC engine.
     * @param paths Number of Monte Carlo paths (default: 10000)
     * @param time_steps Number of exercise opportunities (default: 50)
     * @param seed RNG seed (default: 5489u)
     * @param polynomial_degree Degree of basis polynomials (default: 2)
     * @param vr_method Variance reduction technique (default: None)
     */
    explicit MCAmericanLSMCEngine(std::size_t paths = 10000,
                                  std::size_t time_steps = 50,
                                  unsigned int seed = 5489u,
                                  int polynomial_degree = 2,
                                  VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : MCEngine(paths, seed, vr_method),
          time_steps_(time_steps),
          polynomial_degree_(polynomial_degree) {}

    /**
     * @brief Price an American option using Least-Squares Monte Carlo.
     * @param spec Option specification (must be American exercise style)
     * @param params Market parameters
     * @return PriceOutputs with value, std_dev, and std_error
     * @throws std::invalid_argument if exercise style is not American
     *
     * TODO (v2.0.0): Implement full LSMC algorithm
     */
    PriceOutputs price(const core::OptionSpec& spec,
                       const core::OptionParams& params) const override;

    /**
     * @brief Set the polynomial degree for regression basis.
     * @param degree Degree of Laguerre polynomials (typically 1-3)
     */
    void setPolynomialDegree(int degree) { polynomial_degree_ = degree; }

    int getPolynomialDegree() const { return polynomial_degree_; }

    std::size_t getTimeSteps() const { return time_steps_; }

};  // class MCAmericanLSMCEngine

}  // namespace engines

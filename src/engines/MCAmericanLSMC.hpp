#pragma once

#include <cstddef>

#include "engines/MCEngine.hpp"

namespace engines {

// Longstaff-Schwartz Monte Carlo engine for American vanilla options.
class MCAmericanLSMCEngine : public MCEngine {
   private:
    std::size_t time_steps_;
    int polynomial_degree_;

   public:
   // Constructor
    explicit MCAmericanLSMCEngine(std::size_t paths = 10000, std::size_t time_steps = 50, unsigned int seed = 5489u, int polynomial_degree = 2,
                                  VarianceReductionMethod vr_method = VarianceReductionMethod::None)
        : MCEngine(paths, seed, vr_method), time_steps_(time_steps), polynomial_degree_(polynomial_degree) {}

    PriceOutputs price(const core::OptionSpec& spec, const core::OptionParams& params) const override;
    void setPolynomialDegree(int degree) { polynomial_degree_ = degree; }
    int getPolynomialDegree() const { return polynomial_degree_; }
    std::size_t getTimeSteps() const { return time_steps_; }


};  // class MCAmericanLSMCEngine

}  // namespace engines

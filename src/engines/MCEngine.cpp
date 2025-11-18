#include "engines/MCEngine.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace engines {

std::vector<std::vector<double>> BaseMCEngine::generatePaths(const core::OptionParams& params) const {
    std::size_t steps = std::max<std::size_t>(1, time_steps_);
    std::vector<std::vector<double>> paths(paths_, std::vector<double>(steps + 1, params.S));

    if (paths_ == 0) {
        return paths;
    }

    if (params.T <= 0.0 || params.sig <= 0.0) {
        for (auto& path : paths) {
            std::fill(path.begin(), path.end(), params.S);
        }
        return paths;
    }

    double dt = params.T / static_cast<double>(steps);
    double drift = (params.r - params.q - 0.5 * params.sig * params.sig) * dt;
    double diffusion = params.sig * std::sqrt(dt);

    std::mt19937_64 rng(seed_);
    std::normal_distribution<double> dist(0.0, 1.0);

    if (vr_method_ == VarianceReductionMethod::AntitheticVariates) {
        for (std::size_t i = 0; i < paths_; i += 2) {
            double spot_plus = params.S;
            double spot_minus = params.S;
            for (std::size_t step = 1; step <= steps; ++step) {
                double z = dist(rng);
                spot_plus *= std::exp(drift + diffusion * z);
                paths[i][step] = spot_plus;
                if (i + 1 < paths_) {
                    spot_minus *= std::exp(drift - diffusion * z);
                    paths[i + 1][step] = spot_minus;
                }
            }
            if (i + 1 >= paths_) {
                break;
            }
        }
    } else {
        for (std::size_t i = 0; i < paths_; ++i) {
            double spot = params.S;
            for (std::size_t step = 1; step <= steps; ++step) {
                double z = dist(rng);
                spot *= std::exp(drift + diffusion * z);
                paths[i][step] = spot;
            }
        }
    }

    return paths;
}

void BaseMCEngine::applyVarianceReduction(std::vector<double>& discounted_payoffs,
                                          const core::OptionSpec& spec,
                                          const core::OptionParams& params) const {
    (void)spec;
    (void)params;
    if (vr_method_ != VarianceReductionMethod::AntitheticVariates || discounted_payoffs.size() < 2) {
        return;
    }
    std::vector<double> reduced;
    reduced.reserve((discounted_payoffs.size() + 1) / 2);
    for (std::size_t i = 0; i < discounted_payoffs.size(); i += 2) {
        if (i + 1 < discounted_payoffs.size()) {
            reduced.push_back(0.5 * (discounted_payoffs[i] + discounted_payoffs[i + 1]));
        } else {
            reduced.push_back(discounted_payoffs[i]);
        }
    }
    discounted_payoffs.swap(reduced);
}

}  // namespace engines

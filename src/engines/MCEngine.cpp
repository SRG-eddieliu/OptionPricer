#include "engines/MCEngine.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <limits>
#include <stdexcept>
#include "core/Validation.hpp"
#include "math/Stats.hpp"

namespace engines {

void BaseMCEngine::validateConfiguration() const {
    if (paths_ < 2 || time_steps_ == 0) {
        throw std::invalid_argument("Monte Carlo requires at least two paths and one time step");
    }
    if (vr_method_ == VarianceReductionMethod::QuasiMonteCarlo ||
        vr_method_ == VarianceReductionMethod::Multilevel) {
        throw std::invalid_argument("Requested variance reduction is not implemented");
    }
    const bool paired = vr_method_ == VarianceReductionMethod::AntitheticVariates ||
                        vr_method_ == VarianceReductionMethod::AntitheticMomentMatching;
    if (paired && (paths_ < 4 || paths_%2 != 0)) {
        throw std::invalid_argument("Antithetic sampling requires an even count of at least four paths");
    }
}

double BaseMCEngine::payoffStandardError(const std::vector<double>& payoffs) const {
    if (vr_method_ == VarianceReductionMethod::MomentMatching ||
        vr_method_ == VarianceReductionMethod::AntitheticMomentMatching) {
        // Joint moment normalization couples draws; use independent runs for uncertainty.
        return std::numeric_limits<double>::quiet_NaN();
    }
    return math::stats::standard_error(payoffs);
}

std::vector<std::vector<double>> BaseMCEngine::generatePaths(const core::OptionParams& params) const {
    core::validate_market(params);
    validateConfiguration();
    std::size_t steps = std::max<std::size_t>(1, time_steps_);
    std::vector<std::vector<double>> paths(paths_, std::vector<double>(steps + 1, params.S));

    if (paths_ == 0) {
        return paths;
    }

    if (params.T <= 0.0) {
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

    const bool use_antithetic = vr_method_ == VarianceReductionMethod::AntitheticVariates ||
                                vr_method_ == VarianceReductionMethod::AntitheticMomentMatching;
    const bool use_moment = vr_method_ == VarianceReductionMethod::MomentMatching ||
                            vr_method_ == VarianceReductionMethod::AntitheticMomentMatching;

    if (use_antithetic && !use_moment) {
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
        std::vector<double> noises;
        if (use_moment) {
            std::size_t base_paths = use_antithetic ? (paths_ + 1) / 2 : paths_;
            noises.resize(base_paths * steps);
            for (double& z : noises) {
                z = dist(rng);
            }
            double mean = 0.0;
            for (double z : noises) {
                mean += z;
            }
            mean /= noises.size();
            double var = 0.0;
            for (double z : noises) {
                double centered = z - mean;
                var += centered * centered;
            }
            var = std::sqrt(var / noises.size());
            double inv = (var > 0.0) ? 1.0 / var : 1.0;
            for (double& z : noises) {
                z = (z - mean) * inv;
            }
        }

        std::size_t noise_idx = 0;
        auto next_noise = [&]() -> double {
            if (use_moment) {
                return noises[noise_idx++];
            }
            return dist(rng);
        };

        if (use_antithetic) {
            for (std::size_t i = 0; i < paths_; i += 2) {
                double spot_plus = params.S;
                double spot_minus = params.S;
                for (std::size_t step = 1; step <= steps; ++step) {
                    double z = next_noise();
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
                    double z = next_noise();
                    spot *= std::exp(drift + diffusion * z);
                    paths[i][step] = spot;
                }
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
    const bool use_antithetic = vr_method_ == VarianceReductionMethod::AntitheticVariates ||
                                vr_method_ == VarianceReductionMethod::AntitheticMomentMatching;
    if (!use_antithetic || discounted_payoffs.size() < 2) {
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

#include "engines/MCAmericanLSMC.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

#include "math/Stats.hpp"

namespace engines {

namespace {

std::vector<double> laguerreBasis(double x, int degree) {
    // Generate Laguerre polynomial basis up to given degree at x
    int deg = std::max(0, degree);
    std::vector<double> basis(deg + 1, 0.0);
    if (basis.empty()) {
        return basis;
    }

    basis[0] = 1.0;
    if (deg >= 1) {
        basis[1] = 1.0 - x;
    }
    for (int n = 2; n <= deg; ++n) {
        basis[n] = ((2.0 * n - 1.0 - x) * basis[n - 1] - (n - 1.0) * basis[n - 2]) / static_cast<double>(n);
    }
    return basis;
}

bool solveNormalEquations(std::vector<double>& ata, std::vector<double>& atb, int n) {
    // beta solver via Gaussian elimination with partial pivoting
    const double eps = 1e-12;
    for (int col = 0; col < n; ++col) {
        int pivot = col;
        double max_val = std::fabs(ata[col * n + col]);
        for (int row = col + 1; row < n; ++row) {
            double val = std::fabs(ata[row * n + col]);
            if (val > max_val) {
                max_val = val;
                pivot = row;
            }
        }
        if (max_val < eps) {
            return false;
        }
        if (pivot != col) {
            for (int k = col; k < n; ++k) {
                std::swap(ata[col * n + k], ata[pivot * n + k]);
            }
            std::swap(atb[col], atb[pivot]);
        }

        double pivot_val = ata[col * n + col];
        double inv_pivot = 1.0 / pivot_val;
        for (int k = col; k < n; ++k) {
            ata[col * n + k] *= inv_pivot;
        }
        atb[col] *= inv_pivot;

        for (int row = 0; row < n; ++row) {
            if (row == col) {
                continue;
            }
            double factor = ata[row * n + col];
            for (int k = col; k < n; ++k) {
                ata[row * n + k] -= factor * ata[col * n + k];
            }
            atb[row] -= factor * atb[col];
        }
    }
    return true;
}

std::vector<double> regressContinuation(const std::vector<double>& spots,
                                        const std::vector<double>& discounted_cf,
                                        int degree,
                                        double scale) {
    // Perform least squares regression to find continuation value coefficients
    int deg = std::max(0, degree);
    int cols = deg + 1;
    std::vector<double> coefficients(cols, 0.0);
    if (spots.empty()) {
        return coefficients;
    }

    std::vector<double> ata(cols * cols, 0.0);
    std::vector<double> atb(cols, 0.0);
    double inv_scale = (scale > 1e-12) ? 1.0 / scale : 1.0;

    for (std::size_t i = 0; i < spots.size(); ++i) {
        double x = std::max(spots[i], 0.0) * inv_scale;
        auto basis = laguerreBasis(x, deg);
        for (int r = 0; r < cols; ++r) {
            for (int c = 0; c < cols; ++c) {
                ata[r * cols + c] += basis[r] * basis[c];
            }
            atb[r] += basis[r] * discounted_cf[i];
        }
    }

    if (!solveNormalEquations(ata, atb, cols)) {
        coefficients.assign(cols, 0.0);
        coefficients[0] = math::stats::mean(discounted_cf);
        return coefficients;
    }

    return atb;  // solution stored in RHS vector
}

double evaluateContinuation(double spot, const std::vector<double>& coeffs, int degree, double scale) {
    if (coeffs.empty()) {
        return 0.0;
    }
    double inv_scale = (scale > 1e-12) ? 1.0 / scale : 1.0;
    auto basis = laguerreBasis(std::max(spot, 0.0) * inv_scale, std::max(0, degree));
    double value = 0.0;
    for (std::size_t i = 0; i < coeffs.size() && i < basis.size(); ++i) {
        value += coeffs[i] * basis[i];
    }
    return value;
}

}  // namespace

PriceOutputs MCAmericanLSMCEngine::price(const core::OptionSpec& spec,
                                         const core::OptionParams& params) const {
    // American options only
    if (spec.exercise != core::ExerciseStyle::American) {
        throw std::invalid_argument("MCAmericanLSMCEngine: American exercise style required");
    }

    // Handle edge cases
    if (params.T <= 0.0 || params.sig <= 0.0) {
        PriceOutputs outputs{};
        outputs.value = spec.payoff(params.S);
        return outputs;
    }

    std::size_t steps = std::max<std::size_t>(1, time_steps_);
    double dt = params.T / static_cast<double>(steps);
    double discount = std::exp(-params.r * dt);
    double drift = (params.r - params.q - 0.5 * params.sig * params.sig) * dt;
    double diffusion = params.sig * std::sqrt(dt);
    double scale = params.K > 1e-12 ? params.K : std::max(params.S, 1.0);

    std::mt19937_64 rng(seed_);
    std::normal_distribution<double> standard_normal(0.0, 1.0);

    // Generate paths
    std::vector<std::vector<double>> paths(paths_, std::vector<double>(steps + 1, params.S));
    for (std::size_t i = 0; i < paths_; ++i) {
        for (std::size_t t = 1; t <= steps; ++t) {
            double z = standard_normal(rng);
            double prev = paths[i][t - 1];
            paths[i][t] = prev * std::exp(drift + diffusion * z);
        }
    }

    std::vector<double> cashflows(paths_);
    for (std::size_t i = 0; i < paths_; ++i) {
        cashflows[i] = spec.payoff(paths[i][steps]);
    }

    int degree = std::max(0, polynomial_degree_);
    for (std::size_t step = steps; step-- > 1;) {
        // Discount future cash flows to current time index
        for (double& cf : cashflows) {
            cf *= discount;
        }

        std::vector<double> itm_spots;
        std::vector<double> itm_cf;
        itm_spots.reserve(paths_);
        itm_cf.reserve(paths_);

        for (std::size_t path = 0; path < paths_; ++path) {
            double spot = paths[path][step];
            double intrinsic = spec.payoff(spot);
            if (intrinsic <= 0.0) {
                continue;
            }
            itm_spots.push_back(spot);
            itm_cf.push_back(cashflows[path]);
        }

        if (itm_spots.empty()) {
            continue;
        }

        auto coefficients = regressContinuation(itm_spots, itm_cf, degree, scale);

        for (std::size_t path = 0; path < paths_; ++path) {
            double spot = paths[path][step];
            double intrinsic = spec.payoff(spot);
            if (intrinsic <= 0.0) {
                continue;
            }
            double continuation = evaluateContinuation(spot, coefficients, degree, scale);
            if (intrinsic > continuation) {
                cashflows[path] = intrinsic;
            }
        }
    }

    // Discount from first exercise date to t=0
    for (double& cf : cashflows) {
        cf *= discount;
    }

    double intrinsic_now = spec.payoff(params.S);
    if (intrinsic_now > 0.0) {
        for (double& cf : cashflows) {
            if (intrinsic_now > cf) {
                cf = intrinsic_now;
            }
        }
    }

    applyVarianceReduction(cashflows, spec, params);

    PriceOutputs outputs{};
    outputs.value = math::stats::mean(cashflows);
    outputs.std_dev = math::stats::standard_deviation(cashflows);
    outputs.std_error = math::stats::standard_error(cashflows);
    return outputs;
}

}  // namespace engines

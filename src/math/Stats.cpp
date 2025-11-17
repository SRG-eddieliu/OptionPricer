#include "math/Stats.hpp"

#include <cmath>

namespace math {
namespace stats {

double mean(const std::vector<double>& data) {
    if (data.empty()) {
        return 0.0;
    }
    double sum = 0.0;
    for (double x : data) {
        sum += x;
    }
    return sum / static_cast<double>(data.size());
}

double variance(const std::vector<double>& data) {
    if (data.size() < 2) {
        return 0.0;
    }
    double mu = mean(data);
    double accum = 0.0;
    for (double x : data) {
        double diff = x - mu;
        accum += diff * diff;
    }
    return accum / static_cast<double>(data.size() - 1);
}

double standard_deviation(const std::vector<double>& data) {
    return std::sqrt(variance(data));
}

double standard_error(const std::vector<double>& data) {
    if (data.empty()) {
        return 0.0;
    }
    return standard_deviation(data) / std::sqrt(static_cast<double>(data.size()));
}

} // namespace stats
} // namespace math

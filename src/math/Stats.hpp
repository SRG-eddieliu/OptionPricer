#pragma once

#include <vector>

namespace math {
namespace stats {

double mean(const std::vector<double>& data);
double standard_deviation(const std::vector<double>& data);
double standard_error(const std::vector<double>& data);

double variance(const std::vector<double>& data); // optional for reuse

} // namespace stats
} // namespace math

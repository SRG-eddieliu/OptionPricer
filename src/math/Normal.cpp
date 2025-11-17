#include "math/Normal.hpp"

#include <boost/math/distributions/normal.hpp>

namespace math {
namespace normal {

double n(double x) {
    static const boost::math::normal_distribution<double> dist(0.0, 1.0);
    return boost::math::pdf(dist, x);
}

double N(double x) {
    static const boost::math::normal_distribution<double> dist(0.0, 1.0);
    return boost::math::cdf(dist, x);
}

} // namespace normal
} // namespace math

#pragma once

#include <algorithm>
#include <cstdint>

namespace core {

enum class OptionType { Call, Put };
enum class ExerciseStyle { European, American };
enum class BarrierType { UpAndOut, DownAndOut, UpAndIn, DownAndIn };
enum class ExoticType { ArithmeticAsian, Barrier, Lookback };

struct PlainVanillaPayoff {
    double strike{};
    OptionType type{OptionType::Call};

    double operator()(double ST) const noexcept {
        if (type == OptionType::Call) {
            return std::max(ST - strike, 0.0);
        }
        return std::max(strike - ST, 0.0);
    }
};

struct OptionSpec {
    PlainVanillaPayoff payoff;
    ExerciseStyle exercise{ExerciseStyle::European};
};

struct OptionParams {
    double S{};   // spot price
    double K{};   // strike price
    double r{};   // risk-free rate
    double q{};   // dividend yield
    double sig{}; // volatility
    double T{};   // time to maturity (years)
};

struct PathDependentOptionSpec {
    ExoticType type{ExoticType::ArithmeticAsian};
    OptionType option_type{OptionType::Call};
    double strike{};
    double barrier_level{};
    BarrierType barrier_type{BarrierType::UpAndOut};
};

} // namespace core

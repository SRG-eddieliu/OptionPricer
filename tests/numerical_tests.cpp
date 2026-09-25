#define BOOST_TEST_MODULE NumericalRegression
#include <boost/test/included/unit_test.hpp>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "engines/BSEuropeanAnalytic.hpp"
#include "engines/BinomialCRR.hpp"
#include "engines/TrinomialTree.hpp"
#include "engines/MCEuropean.hpp"
#include "engines/MCAmericanLSMC.hpp"
#include "engines/MCPathDependent.hpp"
#include "math/Stats.hpp"

using namespace core;
using namespace engines;

namespace {
OptionSpec option(OptionType type, double strike = 100,
                  ExerciseStyle style = ExerciseStyle::European) {
    return {{strike, type}, style};
}
void near(double actual, double expected, double tolerance) {
    BOOST_REQUIRE(std::isfinite(actual));
    BOOST_CHECK_SMALL(actual - expected, tolerance);
}
}

BOOST_AUTO_TEST_CASE(analytical_prices_and_parity) {
    BSEuropeanAnalytic bs;
    OptionParams p{100, 100, .05, 0, .2, 1};
    near(bs.price(option(OptionType::Call), p).value, 10.450583572185565, 1e-10);
    near(bs.price(option(OptionType::Put), p).value, 5.573526022256971, 1e-10);
    for (double spot : {60., 100., 150.}) {
        for (double rate : {-.02, .05}) {
            p.S = spot; p.r = rate; p.q = .03;
            double c = bs.price(option(OptionType::Call), p).value;
            double put = bs.price(option(OptionType::Put), p).value;
            double forward = p.S * std::exp(-p.q*p.T) - p.K * std::exp(-p.r*p.T);
            near(c - put, forward, 1e-10);
            BOOST_CHECK_GE(c + 1e-10, std::max(forward, 0.));
            BOOST_CHECK_LE(c, p.S * std::exp(-p.q*p.T) + 1e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE(analytical_greeks) {
    BSEuropeanAnalytic bs;
    OptionParams p{105, 100, .04, .02, .25, 1.5};
    for (auto type : {OptionType::Call, OptionType::Put}) {
        auto spec = option(type);
        auto out = bs.price(spec, p);
        auto bumped = [&](double OptionParams::*field, double h) {
            auto up=p, down=p; up.*field += h; down.*field -= h;
            return (bs.price(spec,up).value-bs.price(spec,down).value)/(2*h);
        };
        near(out.delta, bumped(&OptionParams::S, .001), 1e-7);
        auto up=p, down=p; up.S+=.01; down.S-=.01;
        near(out.gamma, (bs.price(spec,up).value-2*out.value+bs.price(spec,down).value)/.0001, 1e-7);
        near(out.vega, bumped(&OptionParams::sig, .00001), 1e-6);
        near(out.rho, bumped(&OptionParams::r, .00001), 1e-5);
        near(out.theta, -bumped(&OptionParams::T, .00001), 1e-6);
    }
}

BOOST_AUTO_TEST_CASE(zero_volatility_and_expiry) {
    BSEuropeanAnalytic bs; BinomialCRREngine bin(100); TrinomialTreeEngine tri(100);
    MCEuropeanEngine mc(1000);
    OptionParams p{100, 100, .05, .01, 0, 2};
    for (auto type : {OptionType::Call, OptionType::Put}) {
        auto spec = option(type);
        double expected = std::exp(-p.r*p.T)*spec.payoff(p.S*std::exp((p.r-p.q)*p.T));
        for (const PricingEngine* engine : std::vector<const PricingEngine*>{&bs,&bin,&tri,&mc}) {
            near(engine->price(spec,p).value, expected, 1e-10);
            auto expired=p; expired.T=0; expired.S=110;
            near(engine->price(spec,expired).value, spec.payoff(expired.S), 1e-10);
        }
    }
}

BOOST_AUTO_TEST_CASE(invalid_inputs) {
    BSEuropeanAnalytic bs; BinomialCRREngine bin(20); TrinomialTreeEngine tri(20);
    MCEuropeanEngine mc(100);
    OptionParams p{100,100,.05,0,.2,1}; auto spec=option(OptionType::Call);
    for (const PricingEngine* engine : std::vector<const PricingEngine*>{&bs,&bin,&tri,&mc}) {
        for (double OptionParams::*field : {&OptionParams::S,&OptionParams::K,&OptionParams::T,&OptionParams::sig}) {
            auto bad=p; bad.*field=-1;
            BOOST_CHECK_THROW(engine->price(spec,bad), std::invalid_argument);
        }
        auto bad=p; bad.r=std::numeric_limits<double>::quiet_NaN();
        BOOST_CHECK_THROW(engine->price(spec,bad), std::invalid_argument);
        BOOST_CHECK_THROW(engine->price(option(OptionType::Call,90),p), std::invalid_argument);
    }
    BOOST_CHECK_THROW(MCEuropeanEngine(0).price(spec,p), std::invalid_argument);
    BOOST_CHECK_THROW(MCEuropeanEngine(3,1,42,VarianceReductionMethod::AntitheticVariates).price(spec,p), std::invalid_argument);
    BOOST_CHECK_THROW(MCEuropeanEngine(10,1,42,VarianceReductionMethod::QuasiMonteCarlo).price(spec,p), std::invalid_argument);
    auto bad=p; bad.sig=.00001;
    BOOST_CHECK_THROW(bin.price(spec,bad), std::invalid_argument);
    BOOST_CHECK_THROW(tri.price(spec,bad), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(tree_convergence) {
    BSEuropeanAnalytic bs; OptionParams p{100,100,.05,.02,.2,1};
    for (auto type : {OptionType::Call,OptionType::Put}) {
        auto spec=option(type); double ref=bs.price(spec,p).value;
        double b0=BinomialCRREngine(100).price(spec,p).value;
        double b1=BinomialCRREngine(800).price(spec,p).value;
        double t0=TrinomialTreeEngine(100).price(spec,p).value;
        double t1=TrinomialTreeEngine(800).price(spec,p).value;
        BOOST_CHECK_LT(std::abs(b1-ref), std::abs(b0-ref));
        BOOST_CHECK_LT(std::abs(t1-ref), std::abs(t0-ref));
        near(b1,ref,.005); near(t1,ref,.006);
    }
}

BOOST_AUTO_TEST_CASE(american_exercise) {
    OptionParams p{100,100,.05,0,.2,1};
    BinomialCRREngine bin(500); TrinomialTreeEngine tri(500);
    for (const PricingEngine* engine : std::vector<const PricingEngine*>{&bin,&tri}) {
        near(engine->price(option(OptionType::Call,100,ExerciseStyle::American),p).value,
             engine->price(option(OptionType::Call),p).value, 1e-9);
        auto put=option(OptionType::Put,100,ExerciseStyle::American);
        BOOST_CHECK_GE(engine->price(put,p).value,engine->price(option(OptionType::Put),p).value);
        auto zero=p; zero.S=80; zero.sig=0;
        near(engine->price(put,zero).value,20,1e-10);
    }
}

BOOST_AUTO_TEST_CASE(monte_carlo_intervals) {
    OptionParams p{100,100,.05,.02,.2,1}; auto spec=option(OptionType::Call);
    double ref=BSEuropeanAnalytic().price(spec,p).value;
    for (std::uint64_t seed : {7,19,31,43,59,71,83,97}) {
        MCEuropeanEngine mc(40000,1,seed);
        auto out=mc.price(spec,p);
        near(mc.price(spec,p).value,out.value,1e-12);
        BOOST_CHECK_GT(out.std_error,0);
        BOOST_CHECK_LT(std::abs(out.value-ref),5*out.std_error);
        near(out.std_error,out.std_dev/std::sqrt(40000.),1e-12);
    }
}

BOOST_AUTO_TEST_CASE(variance_reduction) {
    OptionParams p{100,100,.05,0,.2,1}; auto spec=option(OptionType::Call);
    std::vector<double> plain,anti,moment;
    for (std::uint64_t seed=1;seed<=32;++seed) {
        plain.push_back(MCEuropeanEngine(6000,1,seed).price(spec,p).value);
        auto a=MCEuropeanEngine(6000,1,seed,VarianceReductionMethod::AntitheticVariates).price(spec,p);
        anti.push_back(a.value);
        near(a.std_error,a.std_dev/std::sqrt(3000.),1e-12);
        auto m=MCEuropeanEngine(6000,1,seed,VarianceReductionMethod::MomentMatching).price(spec,p);
        moment.push_back(m.value);
        BOOST_CHECK(std::isnan(m.std_error));
    }
    BOOST_CHECK_LT(math::stats::variance(anti),math::stats::variance(plain));
    BOOST_CHECK_LT(math::stats::variance(moment),math::stats::variance(plain));
}

BOOST_AUTO_TEST_CASE(lsmc_time_zero) {
    OptionParams p{90,100,.05,0,.3,1};
    auto spec=option(OptionType::Put,100,ExerciseStyle::American);
    double european=MCEuropeanEngine(50000,1,17).price(option(OptionType::Put),p).value;
    auto out=MCAmericanLSMCEngine(50000,1,17).price(spec,p);
    near(out.value,std::max(10.,european),1e-10);
    BOOST_CHECK(std::isnan(out.std_error));
}

BOOST_AUTO_TEST_CASE(lsmc_tree_reference) {
    OptionParams p{100,100,.05,0,.2,1}; auto spec=option(OptionType::Put,100,ExerciseStyle::American);
    double reference=BinomialCRREngine(1000).price(spec,p).value;
    std::vector<double> values;
    for (std::uint64_t seed : {11,37,83}) {
        values.push_back(MCAmericanLSMCEngine(30000,50,seed,2).price(spec,p).value);
    }
    // A discretized in-sample LSMC smoke check, not a rigorous MC confidence interval.
    near(math::stats::mean(values),reference,.22);
}

BOOST_AUTO_TEST_CASE(barrier_parity) {
    OptionParams p{100,100,.05,0,.2,1};
    MCPathDependentEngine path(4000,20,13);
    PathDependentOptionSpec spec{ExoticType::Barrier,OptionType::Call,100,120,BarrierType::UpAndOut};
    double out=path.price(spec,p).value;
    spec.barrier_type=BarrierType::UpAndIn;
    double in=path.price(spec,p).value;
    near(out+in,MCEuropeanEngine(4000,20,13).price(option(OptionType::Call),p).value,1e-10);
}

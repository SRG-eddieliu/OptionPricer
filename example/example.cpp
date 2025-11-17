#include <iomanip>
#include <iostream>

#include "../src/core/Types.hpp"
#include "../src/engines/BSEuropeanAnalytic.hpp"
#include "../src/engines/BinomialCRR.hpp"
#include "../src/engines/MCEuropean.hpp"
#include "../src/engines/TrinomialTree.hpp"

void print_greek(const std::string& label, const engines::PriceOutputs& outputs) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(20) << label << " | "
              << "Value: " << std::setw(10) << outputs.value
              << "  Delta: " << std::setw(10) << outputs.delta
              << "  Gamma: " << std::setw(10) << outputs.gamma << '\n';
}

void print_mc(const std::string& label, const engines::PriceOutputs& outputs) {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(20) << label << " | "
              << "Value: " << std::setw(10) << outputs.value
              << "  StdDev: " << std::setw(10) << outputs.std_dev
              << "  StdErr: " << std::setw(10) << outputs.std_error << '\n';
}

int main() {
    // Market parameters
    core::OptionParams params{100.0, 100.0, 0.05, 0.02, 0.20, 1.0};

    engines::BSEuropeanAnalytic bs_engine;
    engines::BinomialCRREngine binom_engine(2000, 0.0005);
    engines::TrinomialTreeEngine trinomial_engine(2000, 0.0005);
    engines::MCEuropeanEngine mc_engine(50000, 1337u);

    // ===== SCENARIO 1: EUROPEAN OPTIONS =====
    std::cout << "========== SCENARIO 1: EUROPEAN OPTIONS ==========\n\n";

    core::OptionSpec call_euro{{params.K, core::OptionType::Call},
                               core::ExerciseStyle::European};
    core::OptionSpec put_euro{{params.K, core::OptionType::Put},
                              core::ExerciseStyle::European};

    std::cout << "Market Parameters: S=100, K=100, r=5%, q=2%, σ=20%, T=1 year\n\n";

    auto bs_call = bs_engine.price(call_euro, params);
    auto binom_call = binom_engine.price(call_euro, params);
    auto tri_call = trinomial_engine.price(call_euro, params);
    auto mc_call = mc_engine.price(call_euro, params);

    std::cout << "CALL OPTION:\n";
    print_greek("Black-Scholes", bs_call);
    print_greek("Binomial (2000)", binom_call);
    print_greek("Trinomial (2000)", tri_call);
    print_mc("Monte Carlo (50k)", mc_call);

    auto bs_put = bs_engine.price(put_euro, params);
    auto binom_put = binom_engine.price(put_euro, params);
    auto tri_put = trinomial_engine.price(put_euro, params);
    auto mc_put = mc_engine.price(put_euro, params);

    std::cout << "\nPUT OPTION:\n";
    print_greek("Black-Scholes", bs_put);
    print_greek("Binomial (2000)", binom_put);
    print_greek("Trinomial (2000)", tri_put);
    print_mc("Monte Carlo (50k)", mc_put);

    // ===== SCENARIO 2: AMERICAN OPTIONS =====
    std::cout << "\n========== SCENARIO 2: AMERICAN OPTIONS ==========\n\n";

    core::OptionParams no_div_params = params;
    no_div_params.q = 0.0;  // No dividend for clearer early exercise premium

    core::OptionSpec call_amer{{params.K, core::OptionType::Call},
                               core::ExerciseStyle::American};
    core::OptionSpec put_amer{{params.K, core::OptionType::Put},
                              core::ExerciseStyle::American};

    std::cout << "Market Parameters: S=100, K=100, r=5%, q=0% (no dividend), σ=20%, T=1 year\n\n";

    auto binom_amer_call = binom_engine.price(call_amer, no_div_params);
    auto tri_amer_call = trinomial_engine.price(call_amer, no_div_params);

    // Monte Carlo for European call under no-dividend parameters
    core::OptionSpec call_euro_nodiv{{params.K, core::OptionType::Call}, core::ExerciseStyle::European};
    auto mc_call_nodiv = mc_engine.price(call_euro_nodiv, no_div_params);

    std::cout << "AMERICAN CALL (European worth same without dividend):\n";
    print_greek("Binomial (2000)", binom_amer_call);
    print_greek("Trinomial (2000)", tri_amer_call);
    print_mc("Monte Carlo (50k) Euro", mc_call_nodiv);

    auto binom_amer_put = binom_engine.price(put_amer, no_div_params);
    auto tri_amer_put = trinomial_engine.price(put_amer, no_div_params);

    // For comparison, also compute European put prices
    core::OptionSpec put_euro_nodiv{{params.K, core::OptionType::Put},
                                    core::ExerciseStyle::European};
    auto binom_euro_put = binom_engine.price(put_euro_nodiv, no_div_params);
    auto tri_euro_put = trinomial_engine.price(put_euro_nodiv, no_div_params);
    auto mc_euro_put = mc_engine.price(put_euro_nodiv, no_div_params);

    std::cout << "\nAMERICAN PUT (early exercise premium shown):\n";
    print_greek("Binomial Euro", binom_euro_put);
    print_greek("Binomial Amer", binom_amer_put);
    std::cout << "  Early exercise premium (Binomial): " << std::fixed << std::setprecision(6)
              << (binom_amer_put.value - binom_euro_put.value) << '\n';

    print_greek("Trinomial Euro", tri_euro_put);
    print_greek("Trinomial Amer", tri_amer_put);
    print_mc("Monte Carlo (50k) Euro", mc_euro_put);
    std::cout << "  Early exercise premium (Trinomial): " << std::fixed << std::setprecision(6)
              << (tri_amer_put.value - tri_euro_put.value) << '\n';

    // ===== SCENARIO 3: CONVERGENCE COMPARISON =====
    std::cout << "\n========== SCENARIO 3: BINOMIAL vs TRINOMIAL CONVERGENCE ==========\n\n";

    std::cout << "Convergence to Black-Scholes (European Call):\n";
    std::cout << "Reference BS Price: " << std::fixed << std::setprecision(6) << bs_call.value << "\n\n";

    core::OptionSpec call_euro_conv{{params.K, core::OptionType::Call},
                                    core::ExerciseStyle::European};

    std::cout << std::left << std::setw(8) << "Steps"
              << std::setw(20) << "Binomial Error"
              << std::setw(20) << "Trinomial Error" << '\n';
    std::cout << std::string(48, '-') << '\n';

    for (int steps : {25000, 50000, 75000, 100000}) {
        engines::BinomialCRREngine binom_conv(steps, 0.0005);
        engines::TrinomialTreeEngine tri_conv(steps, 0.0005);

        auto binom_price = binom_conv.price(call_euro_conv, params);
        auto tri_price = tri_conv.price(call_euro_conv, params);

        double binom_error = std::abs(binom_price.value - bs_call.value);
        double tri_error = std::abs(tri_price.value - bs_call.value);

        std::cout << std::left << std::setw(8) << steps
                  << std::fixed << std::setprecision(8)
                  << std::setw(20) << binom_error
                  << std::setw(20) << tri_error << '\n';
    }

    return 0;
}

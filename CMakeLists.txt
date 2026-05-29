cmake_minimum_required(VERSION 3.20)

project(OptionPricer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

add_library(option_pricer STATIC
    src/engines/BSEuropeanAnalytic.cpp
    src/engines/BinomialCRR.cpp
    src/engines/MCAmericanLSMC.cpp
    src/engines/MCEngine.cpp
    src/engines/MCEuropean.cpp
    src/engines/MCPathDependent.cpp
    src/engines/TrinomialTree.cpp
    src/math/Normal.cpp
    src/math/Stats.cpp
)

target_include_directories(option_pricer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
)

target_compile_options(option_pricer PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
)

add_executable(option_pricer_demo src/main.cpp)
target_link_libraries(option_pricer_demo PRIVATE option_pricer)

set(OPTION_PRICER_EXAMPLES
    black_scholes_example
    binomial_example
    trinomial_example
    mc_european_example
    mc_american_lsmc_example
    mc_path_exotics_example
    mc_variance_strategies_example
)

foreach(example_name IN LISTS OPTION_PRICER_EXAMPLES)
    add_executable(${example_name} example/${example_name}.cpp)
    target_link_libraries(${example_name} PRIVATE option_pricer)
endforeach()

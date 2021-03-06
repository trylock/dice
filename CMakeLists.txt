cmake_minimum_required(VERSION 3.0)

project(dice)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/test)
set(EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)

set(dice_headers
    ${SRC_DIR}/utils.hpp
    ${SRC_DIR}/logger.hpp
    ${SRC_DIR}/value.hpp
    ${SRC_DIR}/lexer.hpp
    ${SRC_DIR}/parser.hpp
    ${SRC_DIR}/safe.hpp
    ${SRC_DIR}/symbols.hpp
    ${SRC_DIR}/functions.hpp
    ${SRC_DIR}/conversions.hpp
    ${SRC_DIR}/environment.hpp
    ${SRC_DIR}/direct_interpreter.hpp
    ${SRC_DIR}/random_variable.hpp
    ${SRC_DIR}/decomposition.hpp
    ${SRC_DIR}/calculator.hpp
)

set(dice_sources
    ${SRC_DIR}/logger.cpp
    ${SRC_DIR}/parser.cpp
    ${SRC_DIR}/symbols.cpp
    ${SRC_DIR}/conversions.cpp
    ${SRC_DIR}/environment.cpp
    ${SRC_DIR}/calculator.cpp
)

set(dice_cli ${SRC_DIR}/main.cpp)

set(all_tests
    ${EXTERNAL_DIR}/catch.hpp
    ${TESTS_DIR}/main.cpp
    ${TESTS_DIR}/utils_test.cpp
    ${TESTS_DIR}/value_test.cpp
    ${TESTS_DIR}/lexer_test.cpp
    ${TESTS_DIR}/parser_test.cpp
    ${TESTS_DIR}/conversions_test.cpp
    ${TESTS_DIR}/environment_test.cpp
    ${TESTS_DIR}/integration_test.cpp
    ${TESTS_DIR}/random_variable_test.cpp
    ${TESTS_DIR}/decomposition_test.cpp
)

include_directories(
    ${SRC_DIR}
	${EXTERNAL_DIR}
	${EXTERNAL_DIR}/linenoise-ng/include
)

link_directories(
    ${EXTERNAL_DIR}/linenoise-ng/build/$(ConfigurationName)
    ${EXTERNAL_DIR}/linenoise-ng/build
)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -std=gnu++14 -g --coverage")
endif()

add_library(dice ${dice_headers} ${dice_sources})
add_executable(dice_cli ${dice_cli})
add_executable(tests ${all_tests})
add_library(linenoise STATIC
    ${EXTERNAL_DIR}/linenoise-ng/src/ConvertUTF.cpp
    ${EXTERNAL_DIR}/linenoise-ng/src/linenoise.cpp
    ${EXTERNAL_DIR}/linenoise-ng/src/wcwidth.cpp
)

target_link_libraries(dice_cli dice linenoise)
target_link_libraries(tests dice)

# Uncomment this to disable random number generater
# This will remove the "roll()" user function
# It is necessary to use this in order to run Valgrind

# add_definitions(-DDISABLE_RNG) 

# Generate documentation
add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/doxygen.conf)

# Generate code coverage 
add_custom_target(cov 
    COMMAND lcov --capture --directory ${PROJECT_SOURCE_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage.info
    COMMAND genhtml ${CMAKE_BINARY_DIR}/coverage.info --output-directory ${CMAKE_BINARY_DIR}/cov
)
#include "catch.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "environment.hpp"

#include <sstream>
#include <string>

TEST_CASE("Interpret an empty expression", "[dice]")
{
    std::stringstream input{ "" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log };
    auto result = parser.parse();

    REQUIRE(log.empty());
    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 0);
}

TEST_CASE("Interpret an arithmetic expression", "[dice]")
{
    std::stringstream input{ "1 + 2 * 3 / 4 - 5" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log };
    auto result = parser.parse();

    REQUIRE(log.empty());
    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == -3);
}

TEST_CASE("Interpret a dice roll expression", "[dice]")
{
    std::stringstream input{ "1d2d4" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log };
    auto result = parser.parse();

    REQUIRE(log.empty());
    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 8.0));
    REQUIRE(prob.find(2)->second == Approx(5 / 32.0));
    REQUIRE(prob.find(3)->second == Approx(3 / 16.0));
    REQUIRE(prob.find(4)->second == Approx(7 / 32.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 8.0));
    REQUIRE(prob.find(6)->second == Approx(3 / 32.0));
    REQUIRE(prob.find(7)->second == Approx(1 / 16.0));
    REQUIRE(prob.find(8)->second == Approx(1 / 32.0));
}

TEST_CASE("Interpret a function call", "[dice]")
{
    std::stringstream input{ "expectation(1d6)" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log };
    auto result = parser.parse();

    REQUIRE(log.empty());
    REQUIRE(result->type() == dice::type_double::id());
    auto&& value = dynamic_cast<dice::type_double&>(*result).data();
    REQUIRE(value == Approx(3.5));
}
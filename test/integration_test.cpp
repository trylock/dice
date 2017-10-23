#include "catch.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "environment.hpp"

#include <sstream>
#include <string>

void test_error_message(std::stringstream& stream, const std::string& msg)
{
    std::string error_msg;
    std::getline(stream, error_msg);
    // remove line, column number and "error:" string
    auto pos = error_msg.find("error:") + 10;
    while (pos < error_msg.size() && std::isspace(error_msg[pos]))
    {
        ++pos;
    }
    error_msg = error_msg.substr(pos); 
    REQUIRE(error_msg == msg);
}

TEST_CASE("Interpret an empty expression", "[dice]")
{
    std::stringstream input{ "" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
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
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
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
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
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
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(log.empty());
    REQUIRE(result->type() == dice::type_double::id());
    auto&& value = dynamic_cast<dice::type_double&>(*result).data();
    REQUIRE(value == Approx(3.5));
}

TEST_CASE("Interpret an expression even if it starts with invalid symbols", "[dice]")
{
    std::stringstream input{ "* ) 1 + 2 * 3" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 7);

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid token at the beginning of an expression: *");
    test_error_message(errors,  "Invalid token at the beginning of an expression: )");
    REQUIRE(errors.peek() == EOF);
}


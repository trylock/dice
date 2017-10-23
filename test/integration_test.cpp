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

TEST_CASE("Interpret a relational operator in", "[dice]")
{
    std::stringstream input{ "1d6 in [2, 5]" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(2 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 3.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator <", "[dice]")
{
    std::stringstream input{ "1d6 < 3" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator <=", "[dice]")
{
    std::stringstream input{ "1d6 <= 3" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 2.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator ==", "[dice]")
{
    std::stringstream input{ "1d6 == 6" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(0)->second == Approx(5 / 6.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator !=", "[dice]")
{
    std::stringstream input{ "1d6 != 6" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(5 / 6.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 6.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator >=", "[dice]")
{
    std::stringstream input{ "1d6 >= 5" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a relational operator >", "[dice]")
{
    std::stringstream input{ "1d6 > 4" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));

    REQUIRE(log.empty());
}

TEST_CASE("Don't interpret the in operator if the lower bound is invalid expression", "[dice]")
{
    std::stringstream input{ "1d4 in [, 3]" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid operand for the lower bound of operator in");
    test_error_message(errors, "Expected <end of expression>, got ,.");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Don't interpret the in operator if the upper bound is invalid expression", "[dice]")
{
    std::stringstream input{ "1d4 in [1, +]" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid operand for the upper bound of operator in");
    test_error_message(errors, "Expected <end of expression>, got +.");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Don't interpret relational operator if the second operand is invalid", "[dice]")
{
    std::stringstream input{ "1 < +" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 1);

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid operand for <relational operator> '<'");
    test_error_message(errors, "Expected <end of expression>, got +.");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Resume interpreting relational operator after finding a sync symbol", "[dice]")
{
    std::stringstream input{ "1 < * 2" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1));

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid token at the beginning of an addition: *");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Don't interpret the + operator if the second operand is invalid", "[dice]")
{
    std::stringstream input{ "2 + *" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 2);

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid operand for binary operator +");
    test_error_message(errors, "Expected <end of expression>, got *.");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Resume interpreting the + operator if we find a sync symbol", "[dice]")
{
    std::stringstream input{ "2 + [ 3" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 5);

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid token at the beginning of a multiplication: [");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Don't interpret the * operator if the second operand is invalid", "[dice]")
{
    std::stringstream input{ "2 * )" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 2);

    REQUIRE(!log.empty());
    test_error_message(errors, "Invalid operand for binary operator *");
    test_error_message(errors, "Expected <end of expression>, got ).");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Resume interpreting the * operator if we find a sync symbol", "[dice]")
{
    std::stringstream input{ "2 * [ 4" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 8);

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid token at the beginning of a dice roll: [");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Interpret arithmetic expressing with doubles and ints", "[dice]")
{
    std::stringstream input{ "1.5 * 2 + 3 - 0.5" };
    std::stringstream errors;
    
    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_double::id());
    auto&& value = dynamic_cast<dice::type_double&>(*result).data();
    REQUIRE(value == Approx(5.5));

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a function with no arguments", "[dice]")
{
    std::stringstream input{ "one() * 2" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };

    dice::environment env;
    env.add_function("one", dice::user_function([](auto&&, auto&&) 
    { 
        return dice::make<dice::type_int>(1); 
    }));
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 2);

    REQUIRE(log.empty());
}

TEST_CASE("Interpret a function with invalid first argument", "[dice]")
{
    std::stringstream input{ "add(,1,2)" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };

    dice::environment env;
    env.add_function("add", dice::user_function(
        [](auto&& first, auto&&) 
        { 
            using fn = dice::function_traits;
            auto a = fn::arg<dice::type_int>(first);
            auto b = fn::arg<dice::type_int>(first + 1);
            a->data() = a->data() + b->data();
            return std::move(*first);
        }, { dice::type_int::id(), dice::type_int::id() }
    ));
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 3);

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid function parameter 0");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Don't interpret the dice roll operator if its operand is invalid", "[dice]")
{
    std::stringstream input{ "1d)" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };

    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_int::id());
    auto&& value = dynamic_cast<dice::type_int&>(*result).data();
    REQUIRE(value == 1);

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid operand for binary operator D (dice roll)");
    test_error_message(errors,  "Expected <end of expression>, got ).");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Resume interpreting the dice roll operator if we find a sync symbol", "[dice]")
{
    std::stringstream input{ "1d[4" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };

    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_rand_var::id());
    auto&& value = dynamic_cast<dice::type_rand_var&>(*result).data();
    auto&& prob = value.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid token at the beginning of a factor: [");
    REQUIRE(errors.peek() == EOF);
}

TEST_CASE("Resume interpreting an expression in function arguments after finding a sync symbol", "[dice]")
{
    std::stringstream input{ "expectation(+1d4)" };
    std::stringstream errors;

    dice::logger log{ &errors };
    dice::lexer lexer{ &input, &log };

    dice::environment env;
    dice::parser<dice::lexer, dice::logger, dice::environment> parser{ &lexer, &log, &env };
    auto result = parser.parse();

    REQUIRE(result->type() == dice::type_double::id());
    auto&& value = dynamic_cast<dice::type_double&>(*result).data();
    REQUIRE(value == Approx(5 / 2.0));

    REQUIRE(!log.empty());
    test_error_message(errors,  "Invalid token at the beginning of an expression: +");
    REQUIRE(errors.peek() == EOF);
}
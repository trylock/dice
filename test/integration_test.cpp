#include "catch.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "interpreter.hpp"
#include "environment.hpp"

#include <sstream>
#include <string>

using value_list = std::vector<std::unique_ptr<dice::base_value>>;

struct interpreter_result 
{
    // computed values
    value_list values;

    // parser and lexer errors
    std::stringstream errors;

    void assert_no_error()
    {
        REQUIRE(errors.peek() == EOF);
    }

    void assert_error(const std::string& expected_msg) 
    {
        std::string actual_msg; 
        std::getline(errors, actual_msg);
        REQUIRE(actual_msg.size() > 0);
            
        // remove line, column number and "error:" string
        auto pos = actual_msg.find("error:") + 10;
        while (pos < actual_msg.size() && std::isspace(actual_msg[pos]))
        {
            ++pos;
        }
        actual_msg = actual_msg.substr(pos);

        REQUIRE(actual_msg == expected_msg);
    }
};

// Helper function to setup a parser and parse given expression
static interpreter_result interpret(const std::string& expr)
{
    interpreter_result result;

    std::stringstream input{ expr };

    dice::logger log{ &result.errors };
    dice::lexer lexer{ &input, &log };
    dice::environment env;
    
    // functions used in some tests
    env.add_function("one", dice::user_function([](auto&&, auto&&) 
    { 
        return dice::make<dice::type_int>(1); 
    }));

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

    dice::interpreter<dice::environment> interpret{ &env };

    auto parser = dice::make_parser(&lexer, &log, &interpret);

    result.values = parser.parse();
    return result;
}

TEST_CASE("Interpret an empty expression", "[dice]")
{
    auto result = interpret("");
    REQUIRE(result.values.size() == 0);
}

TEST_CASE("Interpret a single integer value", "[dice]")
{
    auto result = interpret("42");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());
    
    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 42);
}

TEST_CASE("Interpret a single double value", "[dice]")
{
    auto result = interpret("3.1415");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_double::id());

    auto data = dynamic_cast<dice::type_double&>(*value).data();
    REQUIRE(data == Approx(3.1415));
}

TEST_CASE("Interpret an invalid double value", "[dice]")
{
    auto result = interpret("3.");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid floating point number. Decimal part must not be empty: 3.");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_double::id());

    auto data = dynamic_cast<dice::type_double&>(*value).data();
    REQUIRE(data == Approx(3.0));
}

TEST_CASE("Skip unknown characters", "[dice]")
{
    auto result = interpret("?!4");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Unexpected character: '?' (0x3F).");
    result.assert_error("Unexpected character: '!' (0x21).");
    result.assert_no_error();
    
    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 4);
}

TEST_CASE("Interpret an arithmetic expression", "[dice]")
{
    auto result = interpret("1 + 2 * 3 / 4 - 5");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto&& data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == -3);
}

TEST_CASE("Interpret a dice roll expression", "[dice]")
{
    auto result = interpret("1d2d4");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
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
    auto result = interpret("expectation(1d6)");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_double::id());

    auto data = dynamic_cast<dice::type_double&>(*value).data();
    REQUIRE(data == Approx(3.5));
}

TEST_CASE("Interpret an expression even if it starts with invalid symbols", "[dice]")
{
    auto result = interpret("* ) 1 + 2 * 3");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid token at the beginning of statement: *");
    result.assert_error( "Invalid token at the beginning of statement: )");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 7);

}

TEST_CASE("Interpret a relational operator in", "[dice]")
{
    auto result = interpret("1d6 in [2, 5]");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(2 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 3.0));
}

TEST_CASE("Interpret a relational operator <", "[dice]")
{
    auto result = interpret("1d6 < 3");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));
}

TEST_CASE("Interpret a relational operator <=", "[dice]")
{
    auto result = interpret("1d6 <= 3");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 2.0));
}

TEST_CASE("Interpret a relational operator ==", "[dice]")
{
    auto result = interpret("1d6 == 6");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(0)->second == Approx(5 / 6.0));
}

TEST_CASE("Interpret a relational operator !=", "[dice]")
{
    auto result = interpret("1d6 != 6");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(5 / 6.0));
    REQUIRE(prob.find(0)->second == Approx(1 / 6.0));
}

TEST_CASE("Interpret a relational operator >=", "[dice]")
{
    auto result = interpret("1d6 >= 5");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));
}

TEST_CASE("Interpret a relational operator >", "[dice]")
{
    auto result = interpret("1d6 > 4");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();
    
    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 3.0));
    REQUIRE(prob.find(0)->second == Approx(2 / 3.0));
}

TEST_CASE("Don't interpret the in operator if the lower bound is invalid expression", "[dice]")
{
    auto result = interpret("1d4 in [, 3]");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid operand for the lower bound of operator in");
    result.assert_error("Expected <end of input>, got ,.");
    result.assert_no_error();
    
    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
}

TEST_CASE("Don't interpret the in operator if the upper bound is invalid expression", "[dice]")
{
    auto result = interpret("1d4 in [1, +]");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid operand for the upper bound of operator in");
    result.assert_error("Expected <end of input>, got +.");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
}

TEST_CASE("Don't interpret relational operator if the second operand is invalid", "[dice]")
{
    auto result = interpret("1 < +");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid operand for <relational operator> '<'");
    result.assert_error("Expected <end of input>, got +.");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 1);
}

TEST_CASE("Resume interpreting relational operator after finding a sync symbol", "[dice]")
{
    auto result = interpret("1 < * 2");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid token at the beginning of addition: *");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1));
}

TEST_CASE("Don't interpret the + operator if the second operand is invalid", "[dice]")
{
    auto result = interpret("2 + *");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid operand for binary operator +");
    result.assert_error("Expected <end of input>, got *.");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 2);
}

TEST_CASE("Resume interpreting the + operator if we find a sync symbol", "[dice]")
{
    auto result = interpret("2 + [ 3");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid token at the beginning of multiplication: [");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 5);
}

TEST_CASE("Don't interpret the * operator if the second operand is invalid", "[dice]")
{
    auto result = interpret("2 * )");

    REQUIRE(result.values.size() == 1);
    result.assert_error("Invalid operand for binary operator *");
    result.assert_error("Expected <end of input>, got ).");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 2);
}

TEST_CASE("Resume interpreting the * operator if we find a sync symbol", "[dice]")
{
    auto result = interpret("2 * [ 4");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid token at the beginning of dice roll: [");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 8);
}

TEST_CASE("Interpret arithmetic expressing with doubles and ints", "[dice]")
{
    auto result = interpret("1.5 * 2 + 3 - 0.5");

    REQUIRE(result.values.size() == 1);
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_double::id());

    auto data = dynamic_cast<dice::type_double&>(*value).data();
    REQUIRE(data == Approx(5.5));
}

TEST_CASE("Interpret a function with no arguments", "[dice]")
{
    auto result = interpret("one() * 2");

    REQUIRE(result.values.size() == 1);
    
    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 2);
}

TEST_CASE("Interpret a function with invalid first argument", "[dice]")
{
    auto result = interpret("add(,1)");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid function parameter 0. Using the default value instead.");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 1); // 0 + 1 
}

TEST_CASE("Don't interpret the dice roll operator if its operand is invalid", "[dice]")
{
    auto result = interpret("1d)");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid operand for binary operator D (dice roll)");
    result.assert_error( "Expected <end of input>, got ).");
    result.assert_no_error();
    
    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_int::id());

    auto data = dynamic_cast<dice::type_int&>(*value).data();
    REQUIRE(data == 1);
}

TEST_CASE("Resume interpreting the dice roll operator if we find a sync symbol", "[dice]")
{
    auto result = interpret("1d[4");
    
    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid token at the beginning of factor: [");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 4.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 4.0));
}

TEST_CASE("Resume interpreting an expression in function arguments after finding a sync symbol", "[dice]")
{
    auto result = interpret("expectation(+1d4)");

    REQUIRE(result.values.size() == 1);
    result.assert_error( "Invalid token at the beginning of expression: +");
    result.assert_no_error();

    auto value = std::move(result.values[0]);
    REQUIRE(value->type() == dice::type_double::id());

    auto data = dynamic_cast<dice::type_double&>(*value).data();
    REQUIRE(data == Approx(5 / 2.0));
}

TEST_CASE("Interpret expression with variables", "[dice]")
{
    auto result = interpret("var X = 1d6; (X == 5) * 4 + (1 - (X == 5)) * 2");

    REQUIRE(result.values.size() == 2);
    result.assert_no_error();

    REQUIRE(result.values[0] == nullptr);
    
    auto value = std::move(result.values[1]);
    REQUIRE(value->type() == dice::type_rand_var::id());

    auto data = dynamic_cast<dice::type_rand_var&>(*value).data();
    auto prob = data.probability();
    REQUIRE(prob.find(4)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(2)->second == Approx(5 / 6.0));
}
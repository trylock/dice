#include "catch.hpp"
#include "lexer.hpp"

#include <sstream>

TEST_CASE("Run lexer on an empty input", "[lexer]")
{
    std::stringstream input;
    std::stringstream error;
    dice::lexer lex{ &input, &error };

    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(error.str().empty());
}

TEST_CASE("Find operator tokens and parentheses", "[lexer]")
{
    std::stringstream input{ " \t\n+ \t \n-*/)(," };
    std::stringstream error;
    dice::lexer lex{ &input, &error };
    REQUIRE(lex.read_token().type == dice::token_type::add);
    REQUIRE(lex.read_token().type == dice::token_type::sub);
    REQUIRE(lex.read_token().type == dice::token_type::mult);
    REQUIRE(lex.read_token().type == dice::token_type::div);
    REQUIRE(lex.read_token().type == dice::token_type::right_parent);
    REQUIRE(lex.read_token().type == dice::token_type::left_parent);
    REQUIRE(lex.read_token().type == dice::token_type::param_delim);
    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(error.str().empty());
}

TEST_CASE("Find relational operator tokens", "[lexer]")
{
    std::stringstream input{ " \t\n<=<!===>>=in\n" };
    std::stringstream error;
    dice::lexer lex{ &input, &error };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == "<=");

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == "<");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == "!=");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == "==");

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == ">");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::rel_op);
    REQUIRE(token.value == ">=");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::in);

    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(error.str().empty());
}

TEST_CASE("Find dice operator", "[lexer]")
{
    std::stringstream input{ " \t \t\nd di D Da D6\t" };
    std::stringstream error;
    dice::lexer lex{ &input, &error };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::token_type::roll_op);
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::id);
    REQUIRE(token.value == "di");

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::roll_op);

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::id);
    REQUIRE(token.value == "Da");

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::roll_op);

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::number);
    REQUIRE(token.value == "6");

    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(error.str().empty());
}

TEST_CASE("Find a number", "[lexer]")
{
    std::stringstream input{ "42 a24 1" };
    std::stringstream error;
    dice::lexer lex{ &input, &error };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::token_type::number);
    REQUIRE(token.value == "42");

    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::id);
    REQUIRE(token.value == "a24");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::token_type::number);
    REQUIRE(token.value == "1");

    REQUIRE(lex.read_token().type == dice::token_type::end);
    REQUIRE(error.str().empty());
}
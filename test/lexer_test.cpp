#include "catch.hpp"
#include "lexer.hpp"

#include <sstream>

TEST_CASE("Run lexer on an empty input", "[lexer]")
{
    std::stringstream input;
    dice::logger log;
    dice::lexer lex{ &input, &log };

    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(log.empty());
}

TEST_CASE("Find operator tokens and parentheses", "[lexer]")
{
    std::stringstream input{ " \t\n+ \t \n-*/)(," };
    dice::logger log;
    dice::lexer lex{ &input, &log };
    REQUIRE(lex.read_token().type == dice::symbol_type::plus);
    REQUIRE(lex.read_token().type == dice::symbol_type::minus);
    REQUIRE(lex.read_token().type == dice::symbol_type::times);
    REQUIRE(lex.read_token().type == dice::symbol_type::divide);
    REQUIRE(lex.read_token().type == dice::symbol_type::right_paren);
    REQUIRE(lex.read_token().type == dice::symbol_type::left_paren);
    REQUIRE(lex.read_token().type == dice::symbol_type::param_delim);
    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(log.empty());
}

TEST_CASE("Find relational operator tokens", "[lexer]")
{
    std::stringstream input{ " \t\n<=<!===>>=in\n" };
    dice::logger log;
    dice::lexer lex{ &input, &log };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == "<=");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == "<");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == "!=");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == "==");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == ">");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == ">=");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::in);

    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(log.empty());
}

TEST_CASE("Find dice operator", "[lexer]")
{
    std::stringstream input{ " \t \t\nd di D Da D6\t" };
    dice::logger log;
    dice::lexer lex{ &input, &log };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::roll_op);
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "di");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::roll_op);

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "Da");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::roll_op);

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "6");

    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(log.empty());
}

TEST_CASE("Find a number", "[lexer]")
{
    std::stringstream input{ "42 a24 1" };
    dice::logger log;
    dice::lexer lex{ &input, &log };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "42");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "a24");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "1");

    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(log.empty());
}

TEST_CASE("Find an integer and a double", "[lexer]")
{
    std::stringstream input{ "0.45 14.0 14 1.001" };
    dice::logger log;
    dice::lexer lex{ &input, &log };

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "0.45");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "14.0");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "14");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "1.001");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::end);
    REQUIRE(log.empty());
}
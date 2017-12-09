#include "catch.hpp"
#include "lexer.hpp"
#include "logger_mock.hpp"

#include <sstream>

namespace 
{
    struct lexer_proxy
    {
        logger_mock logger;
        std::stringstream input;
        dice::lexer<logger_mock> lexer;

        lexer_proxy(const std::string& input_str) : 
            input(input_str), 
            lexer(&input, &logger)
        {
        }

        auto read_token()
        {
            return lexer.read_token();
        }

        auto& errors() const
        {
            return logger.errors();
        }
    };

    auto make_lexer(const std::string& input)
    {
        return lexer_proxy{ input };
    }
}

TEST_CASE("Run lexer on an empty input", "[lexer]")
{
    auto lex = make_lexer("");

    REQUIRE(lex.read_token().type == dice::symbol_type::end);
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Find operator tokens and parentheses", "[lexer]")
{
    auto lex = make_lexer(" \t\n+ \t \n-*/)(,");

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
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Find relational operator tokens", "[lexer]")
{
    auto lex = make_lexer(" \t\n<=<!===>>=in\n");

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
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Find dice operator", "[lexer]")
{
    auto lex = make_lexer(" \t \t\nd di D Da D6\t");

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
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Find a number", "[lexer]")
{
    auto lex = make_lexer("42 a24 1");

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
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Find an integer and a double", "[lexer]")
{
    auto lex = make_lexer("0.45 14.0 14 1.001");

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
    REQUIRE(lex.errors().empty());
}

TEST_CASE("Recognize expressions delimiter", "[lexer]")
{
    auto lex = make_lexer("1 ; 2; 42 ;");

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "1");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::semicolon);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "2");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::semicolon);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "42");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::semicolon);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::end);
    REQUIRE(token.lexeme == "");
}

TEST_CASE("Recognize assignment", "[lexer]")
{
    auto lex = make_lexer("val = 1; ===");
    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "val");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::assign);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::number);
    REQUIRE(token.lexeme == "1");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::semicolon);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::rel_op);
    REQUIRE(token.lexeme == "==");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::assign);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::end);
    REQUIRE(token.lexeme == "");
}

TEST_CASE("Recognize var keyword", "[lexer]")
{
    auto lex = make_lexer("var variable");

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::var);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "variable");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::end);
    REQUIRE(token.lexeme == "");
}

TEST_CASE("Distinguish function and variable identifiers", "[lexer]")
{
    auto lex = make_lexer("id  id \t \n () id()");

    auto token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::id);
    REQUIRE(token.lexeme == "id");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::func_id);
    REQUIRE(token.lexeme == "id");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::left_paren);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::right_paren);
    REQUIRE(token.lexeme == "");
    
    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::func_id);
    REQUIRE(token.lexeme == "id");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::left_paren);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::right_paren);
    REQUIRE(token.lexeme == "");

    token = lex.read_token();
    REQUIRE(token.type == dice::symbol_type::end);
}

TEST_CASE("Multiple decimal parts in one number", "[lexer]")
{
    auto lex = make_lexer("1.2.3");

    auto token = lex.read_token();

    REQUIRE(lex.errors().size() == 1);
    REQUIRE(lex.errors()[0].message == "Malformed number: '1.2.3'");

    REQUIRE(token.lexeme == "1.2");
    REQUIRE(token.type == dice::symbol_type::number);
}

TEST_CASE("Missing decimal part of a number", "[lexer]")
{
    auto lex = make_lexer("3.");

    auto token = lex.read_token();

    REQUIRE(lex.errors().size() == 1);
    REQUIRE(lex.errors()[0].message == "Malformed number: '3.'");

    REQUIRE(token.lexeme == "3.0");
    REQUIRE(token.type == dice::symbol_type::number);
}
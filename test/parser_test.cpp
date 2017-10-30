#include "catch.hpp"
#include "parser.hpp"

#include "logger_mock.hpp"

using symbols = std::vector<dice::symbol>;

class lexer_mock 
{
public:
    explicit lexer_mock(std::vector<dice::symbol>&& tokens) : 
        tokens_(std::move(tokens)), pos_(0) {}

    dice::symbol read_token() 
    {
        if (pos_ >= tokens_.size())
            return dice::symbol{ dice::symbol_type::end };
        return std::move(tokens_[pos_++]);
    }

    dice::lexer_location location()
    {
        return dice::lexer_location{ 0, 0 };
    }
private:
    std::vector<dice::symbol> tokens_;
    std::size_t pos_;
};

// simple interpreter mock that will translate the expression to more explicit form
class interpreter_mock
{
public:
    using value_type = std::string;
    using value_list = std::vector<value_type>;

    void enter_assign() {}
    void leave_assign() {}

    value_type make_default()
    {
        return "<DEFAULT>";
    }

    value_type number(const std::string& value)
    {    
        return value;
    }

    value_type variable(const std::string& name)
    {
        if (name == "E")
        {
            throw dice::compiler_error("Unknown variable 'E'");
        }
        return name;
    }

    value_type add(value_type&& lhs, value_type&& rhs)
    {
        return "(" + lhs + "+" + rhs + ")";
    }

    value_type sub(value_type&& lhs, value_type&& rhs)
    {
        return "(" + lhs + "-" + rhs + ")";
    }

    value_type mult(value_type&& lhs, value_type&& rhs)
    {
        return "(" + lhs + "*" + rhs + ")";
    }

    value_type div(value_type&& lhs, value_type&& rhs)
    {
        return "(" + lhs + "/" + rhs + ")";
    }

    value_type unary_minus(value_type&& value)
    {
        return "(-" + value + ")";
    }

    value_type rel_op(const std::string& type, value_type&& lhs, 
        value_type&& rhs)
    {
        return "(" + lhs + type + rhs + ")";
    }

    value_type rel_in(value_type&& var, value_type&& lower_bound, 
        value_type&& upper_bound)
    {
        return "(" + var + " in[" + lower_bound + "," + upper_bound + "])";
    }

    value_type roll(value_type&& lhs, value_type&& rhs)
    {
        return "(" + lhs + "d" + rhs + ")";
    }

    value_type assign(const std::string& name, value_type&& value)
    {
        return "(" + name + "=" + value + ");";
    }

    value_type call(const std::string& name, value_list&& args)
    {
        if (name == "E")
        {
            throw dice::compiler_error("Unknown function 'E'");
        }

        std::string trans = name + "(";
        if (args.size() == 0)
        {
            trans += ")";
            return trans;
        }
        trans += args.front();
        for (auto it = args.begin() + 1; it != args.end(); ++it)
        {
            trans += "," + *it;
        }
        return trans + ")";
    }

    void terminal(const dice::symbol&) {}
};

struct parse_result
{
    std::vector<log_entry> errors;
    interpreter_mock::value_list values;
};

static auto parse(std::vector<dice::symbol>&& tokens)
{
    lexer_mock lexer{ std::move(tokens) };
    logger_mock logger;
    interpreter_mock interpret;
    dice::parser<lexer_mock, logger_mock, interpreter_mock> parser{ 
        &lexer, &logger, &interpret };
    
    parse_result result;
    result.values = parser.parse();
    result.errors = std::move(logger.errors());
    return result;
}

TEST_CASE("Parse an empty expression", "[parser]")
{
    auto result = parse({});

    REQUIRE(result.values.size() == 0);
}

TEST_CASE("Parse simple expression", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "14" }
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "14");
}

TEST_CASE("Operators + and - are left associative", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::plus },
        { symbol_type::number, "2" },
        { symbol_type::minus },
        { symbol_type::number, "3" },
        { symbol_type::plus },
        { symbol_type::number, "4" },
        { symbol_type::minus },
        { symbol_type::number, "5" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1+2)-3)+4)-5)");
}

TEST_CASE("Operators * and / are left associative", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::times },
        { symbol_type::number, "2" },
        { symbol_type::divide },
        { symbol_type::number, "3" },
        { symbol_type::times },
        { symbol_type::number, "4" },
        { symbol_type::divide },
        { symbol_type::number, "5" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1*2)/3)*4)/5)");
}

TEST_CASE("Operator D (dice roll) is left associative", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::roll_op },
        { symbol_type::number, "2" },
        { symbol_type::roll_op },
        { symbol_type::number, "3" },
        { symbol_type::roll_op },
        { symbol_type::number, "4" },
        { symbol_type::roll_op },
        { symbol_type::number, "5" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1d2)d3)d4)d5)");
}

TEST_CASE("Operators * and / have higher precedence than + and -", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::plus },
        { symbol_type::number, "2" },
        { symbol_type::times },
        { symbol_type::number, "3" },
        { symbol_type::minus },
        { symbol_type::number, "4" },
        { symbol_type::divide },
        { symbol_type::number, "5" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1+(2*3))-(4/5))");
}

TEST_CASE("Operator D has higher precedence than unary -", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::minus },
        { symbol_type::number, "1" },
        { symbol_type::roll_op },
        { symbol_type::number, "2" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(-(1d2))");
}

TEST_CASE("Operator D has higher precedence than +, -, * and /", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::plus },
        { symbol_type::number, "2" },
        { symbol_type::roll_op },
        { symbol_type::number, "3" },
        { symbol_type::times },
        { symbol_type::number, "4" },
        { symbol_type::roll_op },
        { symbol_type::number, "5" },
        { symbol_type::minus },
        { symbol_type::number, "6" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1+((2d3)*(4d5)))-6)");
}

TEST_CASE("Operator D has higher precedence than a relational operator", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::roll_op },
        { symbol_type::number, "2" },
        { symbol_type::rel_op, "<" },
        { symbol_type::number, "3" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1d2)<3)");
}

TEST_CASE("Operator D has higher precedence than in operator", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::roll_op },
        { symbol_type::number, "2" },
        { symbol_type::in },
        { symbol_type::left_square_bracket },
        { symbol_type::number, "3" },
        { symbol_type::param_delim },
        { symbol_type::number, "4" },
        { symbol_type::right_square_bracket },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1d2) in[3,4])");
}

TEST_CASE("Assign operator has the lowest precedence", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::var },
        { symbol_type::id, "X" },
        { symbol_type::assign },
        { symbol_type::number, "1" },
        { symbol_type::rel_op, "<" },
        { symbol_type::number, "2" },
        { symbol_type::plus },
        { symbol_type::number, "3" },
        { symbol_type::minus },
        { symbol_type::number, "4" },
        { symbol_type::times },
        { symbol_type::number, "5" },
        { symbol_type::divide },
        { symbol_type::minus },
        { symbol_type::number, "6" },
        { symbol_type::roll_op },
        { symbol_type::number, "7" },
    });

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(X=(1<((2+3)-((4*5)/(-(6d7))))));");
}

TEST_CASE("Parse erroneous second operand for a relational operator", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::rel_op, "<" },
        { symbol_type::assign },
        { symbol_type::number, "2" }
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(1<2)");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of addition: =");
}

TEST_CASE("Parse invalid tokens at the beginning of an expression", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::left_square_bracket },
        { symbol_type::plus },
        { symbol_type::number, "1" },
        { symbol_type::plus },
        { symbol_type::number, "2" }
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(1+2)");

    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of statement: [");
    REQUIRE(result.errors[1].message == "Invalid token at the beginning of statement: +");
}

TEST_CASE("Parse an empty statement in statements list", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::number, "1" },
        { symbol_type::semicolon },
        { symbol_type::right_paren },
        { symbol_type::semicolon },
        { symbol_type::number, "2" },
    });
    
    REQUIRE(result.values.size() == 2);
    REQUIRE(result.values[0] == "1");
    REQUIRE(result.values[1] == "2");

    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of statement: )");
    REQUIRE(result.errors[1].message == "Invalid statement.");
}

TEST_CASE("Use default value instead of a nonexistent variable name", "[parser]")
{
    using namespace dice;

    // E is a special variable name for which the interpreter mock throws
    // a compiler error simulating a nonexitent variable
    auto result = parse(symbols{
        { symbol_type::id, "E" }, 
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Unknown variable 'E'");
}

TEST_CASE("Use default value instead of an invalid expression in name definition", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::var },
        { symbol_type::id, "X" },
        { symbol_type::assign },
        { symbol_type::semicolon },
    });
    
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(X=<DEFAULT>);");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Invalid expression. Using the default value instead.");
}

TEST_CASE("If an error occurs during a function call, use default value and provide an error message", "[parser]")
{
    using namespace dice;

    // E is a special function name for which the interpreter mock throws
    // a compiler error simulating a nonexistent function
    auto result = parse(symbols{
        { symbol_type::id, "E" },
        { symbol_type::left_paren },
        { symbol_type::right_paren },
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Unknown function 'E'");
}

TEST_CASE("Replace invalid function arguments with the default value", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::id, "func" },
        { symbol_type::left_paren },
        { symbol_type::assign },
        { symbol_type::param_delim },
        { symbol_type::number, "1" },
        { symbol_type::right_paren },
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "func(<DEFAULT>,1)");
    
    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of expression: =");
    REQUIRE(result.errors[1].message == "Invalid function parameter 0. Using the default value instead.");
}

TEST_CASE("Replace invalid parenthesised expression with the default value", "[parser]")
{
    using namespace dice;

    auto result = parse(symbols{
        { symbol_type::left_paren },
        { symbol_type::left_square_bracket },
        { symbol_type::right_square_bracket },
        { symbol_type::right_paren },
    });

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 3);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of expression: [");
    REQUIRE(result.errors[1].message == "Invalid token at the beginning of expression: ]");
    REQUIRE(result.errors[2].message == "Invalid expression. Using the default value instead.");
}
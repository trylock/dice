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
        return dice::lexer_location{ 0, static_cast<int>(pos_) };
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

    value_type make_default()
    {
        return "<DEFAULT>";
    }

    value_type number(dice::symbol& token)
    {    
        auto int_value = dynamic_cast<dice::type_int*>(token.value.get());
        auto real_value = dynamic_cast<dice::type_real*>(token.value.get());
        if (int_value != nullptr)
        {
            return std::to_string(int_value->data());
        }
        else if (real_value != nullptr)
        {
            return std::to_string(real_value->data());
        }
        assert(false && "Number has to have an int or a real value.");
        return "";
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
        if (lhs == "OVERFLOW" || rhs == "OVERFLOW")
        {
            throw dice::compiler_error("OVERFLOW");
        }
        return "(" + lhs + "+" + rhs + ")";
    }

    value_type sub(value_type&& lhs, value_type&& rhs)
    {
        if (lhs == "OVERFLOW" || rhs == "OVERFLOW")
        {
            throw dice::compiler_error("OVERFLOW");
        }
        return "(" + lhs + "-" + rhs + ")";
    }

    value_type mult(value_type&& lhs, value_type&& rhs)
    {
        if (lhs == "OVERFLOW" || rhs == "OVERFLOW")
        {
            throw dice::compiler_error("OVERFLOW");
        }
        return "(" + lhs + "*" + rhs + ")";
    }

    value_type div(value_type&& lhs, value_type&& rhs)
    {
        if (rhs == "ZERO")
        {
            throw dice::compiler_error("DIVIDE BY ZERO");
        }
        return "(" + lhs + "/" + rhs + ")";
    }

    value_type unary_minus(value_type&& value)
    {
        if (value == "OVERFLOW")
        {
            throw dice::compiler_error("OVERFLOW");
        }
        return "(-" + value + ")";
    }

    value_type rel_op(const std::string& type, value_type&& lhs, 
        value_type&& rhs)
    {
        if (type == "E")
        {
            throw dice::compiler_error("ERROR");
        }
        return "(" + lhs + type + rhs + ")";
    }

    value_type rel_in(value_type&& var, value_type&& lower_bound, 
        value_type&& upper_bound)
    {
        if (var == "ERROR" || lower_bound == "ERROR" || upper_bound == "ERROR")
        {
            throw dice::compiler_error("ERROR");
        }
        return "(" + var + " in[" + lower_bound + "," + upper_bound + "])";
    }

    value_type roll(value_type&& lhs, value_type&& rhs)
    {
        if (lhs == "OVERFLOW" || rhs == "OVERFLOW")
        {
            throw dice::compiler_error("OVERFLOW");
        }
        return "(" + lhs + "d" + rhs + ")";
    }

    value_type assign(const std::string& name, value_type&& value)
    {
        if (name == "x")
        {
            throw dice::compiler_error("Variable 'x' redefinition.");
        }
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

template<typename T, std::size_t n>
static auto to_vector(T(&array)[n])
{
    return std::vector<T>{
        std::make_move_iterator(std::begin(array)),
        std::make_move_iterator(std::end(array))
    }; 
}

TEST_CASE("Parse an empty expression", "[parser]")
{
    auto result = parse({});

    REQUIRE(result.values.size() == 0);
}

TEST_CASE("Parse simple expression", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(14) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "14");
}

TEST_CASE("Operators + and - are left associative", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(5) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1+2)-3)+4)-5)");
}

TEST_CASE("Operators * and / are left associative", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::times },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::divide },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::times },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::divide },
        { symbol_type::number, make<type_int>(5) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1*2)/3)*4)/5)");
}

TEST_CASE("Operator D (dice roll) is left associative", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(5) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((((1d2)d3)d4)d5)");
}

TEST_CASE("Operators * and / have higher precedence than + and -", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::times },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::divide },
        { symbol_type::number, make<type_int>(5) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1+(2*3))-(4/5))");
}

TEST_CASE("Operator D has higher precedence than unary -", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(2) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(-(1d2))");
}

TEST_CASE("Operator D has higher precedence than +, -, * and /", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::times },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(5) },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(6) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1+((2d3)*(4d5)))-6)");
}

TEST_CASE("Operator D has higher precedence than a relational operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::rel_op, "<" },
        { symbol_type::number, make<type_int>(3) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1d2)<3)");
}

TEST_CASE("Operator D has higher precedence than in operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::in },
        { symbol_type::left_square_bracket },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::param_delim },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::right_square_bracket },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "((1d2) in[3,4])");
}

TEST_CASE("Assign operator has the lowest precedence", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::var },
        { symbol_type::id, "X" },
        { symbol_type::assign },
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::rel_op, "<" },
        { symbol_type::number, make<type_int>(2) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(3) },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(4) },
        { symbol_type::times },
        { symbol_type::number, make<type_int>(5) },
        { symbol_type::divide },
        { symbol_type::minus },
        { symbol_type::number, make<type_int>(6) },
        { symbol_type::roll_op },
        { symbol_type::number, make<type_int>(7) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.errors.empty());
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(X=(1<((2+3)-((4*5)/(-(6d7))))));");
}

TEST_CASE("Parse erroneous second operand for a relational operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::rel_op, "<" },
        { symbol_type::assign },
        { symbol_type::number, make<type_int>(2) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(1<2)");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of addition: =");
}

TEST_CASE("Parse invalid tokens at the beginning of an expression", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::left_square_bracket },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::plus },
        { symbol_type::number, make<type_int>(2) },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "(1+2)");

    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of statement: [");
    REQUIRE(result.errors[1].message == "Invalid token at the beginning of statement: +");
}

TEST_CASE("Parse an empty statement in statements list", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::semicolon },
        { symbol_type::right_paren },
        { symbol_type::semicolon },
        { symbol_type::number, make<type_int>(2) },
    };
    auto result = parse(to_vector(input));
    
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
    dice::symbol input[] = {
        { symbol_type::id, "E" }, 
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 0);
    REQUIRE(result.errors[0].message == "Unknown variable 'E'");
}

TEST_CASE("Use default value instead of an invalid expression in name definition", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::var },
        { symbol_type::id, "X" },
        { symbol_type::assign },
        { symbol_type::semicolon },
    };
    auto result = parse(to_vector(input));
    
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
    dice::symbol input[] = {
        { symbol_type::func_id, "E" },
        { symbol_type::left_paren },
        { symbol_type::right_paren },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 0);
    REQUIRE(result.errors[0].message == "Unknown function 'E'");
}

TEST_CASE("Replace invalid function arguments with the default value", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::func_id, "func" },
        { symbol_type::left_paren },
        { symbol_type::assign },
        { symbol_type::param_delim },
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::right_paren },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "func(<DEFAULT>,1)");
    
    REQUIRE(result.errors.size() == 2);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of expression: =");
    REQUIRE(result.errors[1].message == "Invalid function parameter 0. Using the default value instead.");
}

TEST_CASE("Replace invalid parenthesised expression with the default value", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::left_paren },
        { symbol_type::left_square_bracket },
        { symbol_type::right_square_bracket },
        { symbol_type::right_paren },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 3);
    REQUIRE(result.errors[0].message == "Invalid token at the beginning of expression: [");
    REQUIRE(result.errors[1].message == "Invalid token at the beginning of expression: ]");
    REQUIRE(result.errors[2].message == "Invalid expression. Using the default value instead.");
}

TEST_CASE("Handle exceptions in assignment", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::var },
        { symbol_type::id, "x" },
        { symbol_type::assign },
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::semicolon },
        { symbol_type::id, "x" },
    };
    auto result = parse(to_vector(input));
    
    REQUIRE(result.values.size() == 2);
    REQUIRE(result.values[0] == "1");
    REQUIRE(result.values[1] == "x");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].message == "Variable 'x' redefinition.");
}

TEST_CASE("Report errors in addition", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::id, "OVERFLOW" },
        { symbol_type::plus },
        { symbol_type::id, "OVERFLOW" },
    };
    auto result = parse(to_vector(input));
    
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "OVERFLOW");
}

TEST_CASE("Report errors in division", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::divide },
        { symbol_type::id, "ZERO" },
    };
    auto result = parse(to_vector(input));
    
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "DIVIDE BY ZERO");
}

TEST_CASE("Report errors in dice roll operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::roll_op },
        { symbol_type::id, "OVERFLOW" },
    };
    auto result = parse(to_vector(input));
    
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "OVERFLOW");
}

TEST_CASE("Report errors in unary minus", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::minus },
        { symbol_type::id, "OVERFLOW" },
    };
    auto result = parse(to_vector(input));
    
    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");
    
    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 0);
    REQUIRE(result.errors[0].message == "OVERFLOW");
}

TEST_CASE("Report errors in binary minus", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::id, "OVERFLOW" },
        { symbol_type::minus },
        { symbol_type::id, "OVERFLOW" },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "OVERFLOW");
}

TEST_CASE("Report errors in multiplication operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::id, "OVERFLOW" },
        { symbol_type::times },
        { symbol_type::id, "OVERFLOW" },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "OVERFLOW");
}


TEST_CASE("Report errors in relational operator", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::number, make<type_int>(1) },
        { symbol_type::rel_op, "E" },
        { symbol_type::number, make<type_int>(2) }  
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "1");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "ERROR");
}

TEST_CASE("Report errors in operator in", "[parser]")
{
    using namespace dice;

    dice::symbol input[] = {
        { symbol_type::id, "ERROR" },
        { symbol_type::in },
        { symbol_type::left_square_bracket },
        { symbol_type::id, "ERROR" },
        { symbol_type::param_delim },
        { symbol_type::id, "ERROR" },
        { symbol_type::right_square_bracket },
    };
    auto result = parse(to_vector(input));

    REQUIRE(result.values.size() == 1);
    REQUIRE(result.values[0] == "<DEFAULT>");

    REQUIRE(result.errors.size() == 1);
    REQUIRE(result.errors[0].line == 0);
    REQUIRE(result.errors[0].col == 1);
    REQUIRE(result.errors[0].message == "ERROR");
}
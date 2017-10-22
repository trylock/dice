#include "catch.hpp"
#include "parser.hpp"

class lexer_mock 
{
public:
    explicit lexer_mock(std::initializer_list<dice::token> tokens) :
        tokens_(tokens.begin(), tokens.end()), pos_(0) {}

    dice::token read_token() 
    {
        if (pos_ >= tokens_.size())
            return dice::token_type::end;
        return tokens_[pos_++];
    }

    dice::lexer_location location()
    {
        return dice::lexer_location{ 0, 0 };
    }
private:
    std::vector<dice::token> tokens_;
    std::size_t pos_;
};

struct log_entry 
{
    int line;
    int col;
    std::string message;
};

class logger_mock 
{
public:
    void error(int line, int col, const std::string& message)
    {
        errors_.push_back(log_entry{
            line,
            col,
            message
        });
    }

    const std::vector<log_entry>& errors() const { return errors_;  }

private:
    std::vector<log_entry> errors_;
};

template<typename Arg>
class env_mock 
{
public:
    using value_type = std::unique_ptr<dice::base_value>;
    using args_iterator = std::vector<value_type>::iterator;

    value_type call_var(const std::string&, args_iterator, args_iterator)
    {
        throw std::runtime_error("Not supported.");
    }

    value_type call(const std::string& name, value_type&& first) 
    {
        assert(name == "unary-");
        auto a = dynamic_cast<Arg*>(first.get());
        a->data() = -a->data();
        return std::move(first);
    }

    value_type call(const std::string& name, value_type&& first, value_type&& second)
    {
        assert(first->type() == second->type());

        auto a = dynamic_cast<Arg*>(first.get());
        auto b = dynamic_cast<Arg*>(second.get());
        if (name == "+")
            a->data() = a->data() + b->data();
        else if (name == "-")
            a->data() = a->data() - b->data();
        else if (name == "*")
            a->data() = a->data() * b->data();
        else if (name == "/")
            a->data() = a->data() / b->data();
        else if (name == "roll")
            first = dice::make<dice::type_rand_var>(
                std::make_pair(1, 1),
                std::make_pair(2, 1),
                std::make_pair(3, 1),
                std::make_pair(4, 1),
                std::make_pair(5, 1),
                std::make_pair(6, 1)
            );
        return std::move(first);
    }

    value_type call(const std::string&, value_type&&, value_type&&, value_type&&) 
    {
        throw std::runtime_error("Not supported.");
    }
};

TEST_CASE("Parse empty expression", "[parser]")
{
    lexer_mock lexer{};
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == 0);
}

TEST_CASE("Parse simple expression", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::left_parent },
        { dice::token_type::number, "1" },
        { dice::token_type::add },
        { dice::token_type::number, "2" },
        { dice::token_type::right_parent },
        { dice::token_type::mult },
        { dice::token_type::number, "3" },
        { dice::token_type::div },
        { dice::token_type::number, "5" },
        { dice::token_type::add },
        { dice::token_type::number, "1" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == 2);
}

TEST_CASE("+ and - operators are left associative", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::number, "1" },
        { dice::token_type::add },
        { dice::token_type::number, "2" },
        { dice::token_type::sub },
        { dice::token_type::number, "3" },
        { dice::token_type::add },
        { dice::token_type::number, "4" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == 4);
}

TEST_CASE("* and / operators are left associative", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::number, "2" },
        { dice::token_type::mult },
        { dice::token_type::number, "3" },
        { dice::token_type::div },
        { dice::token_type::number, "4" },
        { dice::token_type::mult },
        { dice::token_type::number, "5" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == 5);
}

TEST_CASE("* and / operators have higher precedence than + and -", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::number, "1" },
        { dice::token_type::add },
        { dice::token_type::number, "2" },
        { dice::token_type::mult },
        { dice::token_type::number, "3" },
        { dice::token_type::sub },
        { dice::token_type::number, "4" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == 3);
}

TEST_CASE("Unary - has higher precedence than +, -, * and /", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::number, "1" },
        { dice::token_type::add },
        { dice::token_type::number, "2" },
        { dice::token_type::mult },
        { dice::token_type::sub },
        { dice::token_type::number, "3" },
        { dice::token_type::add },
        { dice::token_type::sub },
        { dice::token_type::number, "4" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_int::id());
    REQUIRE(dynamic_cast<dice::type_int*>(result.get())->data() == -9);
}

TEST_CASE("Parse dice roll operator", "[parser]")
{
    lexer_mock lexer{
        { dice::token_type::number, "1" },
        { dice::token_type::roll_op },
        { dice::token_type::number, "6" },
    };
    logger_mock log;
    dice::parser<lexer_mock, logger_mock, env_mock<dice::type_int>> parser{ &lexer, &log };
    auto result = parser.parse();
    REQUIRE(log.errors().empty());
    REQUIRE(result->type() == dice::type_rand_var::id());
}

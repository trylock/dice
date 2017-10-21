#include "parser.hpp"

// dice expression parser

dice::parser::parser(lexer* l) : lexer_(l)
{
}

dice::parser::value_type dice::parser::parse()
{
    lookahead_ = lexer_->read_token();
    auto result = expr();
    eat(token_type::end); // make sure we've processed the whole expression
    return result;
}

dice::parser::value_type dice::parser::expr()
{
    auto left = add();
    if (lookahead_.type == token_type::in)
    {
        eat(token_type::in);
        eat(token_type::left_square_bracket);
        auto lower_bound = expr();
        eat(token_type::param_delim);
        auto upper_bound = expr();
        eat(token_type::right_square_bracket);

        return environment_.call("in", 
            std::move(left), 
            std::move(lower_bound), 
            std::move(upper_bound));
    }
    else if (lookahead_.type == token_type::rel_op)
    {
        auto op = lookahead_;
        eat(token_type::rel_op);
        auto right = add();

        // calculate the value
        return environment_.call(op.value, std::move(left), std::move(right));
    }
    return left;
}

dice::parser::value_type dice::parser::add()
{
    auto result = mult();
    for (;;)
    {
        if (lookahead_.type == token_type::add)
        {
            eat(token_type::add);
            result = environment_.call("+", std::move(result), mult());
        }
        else if (lookahead_.type == token_type::sub)
        {
            eat(token_type::sub);
            result = environment_.call("-", std::move(result), mult());
        }
        else 
        {
            break;
        }
    }
    return result;
}

dice::parser::value_type dice::parser::mult()
{
    auto result = dice_roll();
    for (;;)
    {
        if (lookahead_.type == token_type::mult)
        {
            eat(token_type::mult);
            result = environment_.call("*", std::move(result), dice_roll());
        }
        else if (lookahead_.type == token_type::div)
        {
            eat(token_type::div);
            result = environment_.call("/", std::move(result), dice_roll());
        }
        else 
        {
            break;
        }
    }
    return result;
}

dice::parser::value_type dice::parser::dice_roll()
{
    // determine the sign
    int count = 0;
    for (;;)
    {
        if (lookahead_.type == token_type::sub)
        {
            eat(token_type::sub);
            ++count;
        }
        else 
        {
            break;
        }
    }

    // parse dice roll
    auto result = factor();
    for (;;)
    {
        if (lookahead_.type == token_type::roll_op)
        {
            eat(token_type::roll_op);
            result = environment_.call("roll", std::move(result), factor());
        }
        else 
        {
            break;
        }
    }

    // add sign
    if (count % 2 != 0)
        result = environment_.call("unary-", std::move(result));
    return result; 
}

dice::parser::value_type dice::parser::factor()
{
    if (lookahead_.type == token_type::left_parent)
    {
        eat(token_type::left_parent);
        auto result = expr();
        eat(token_type::right_parent);
        return result;
    }
    else if (lookahead_.type == token_type::number)
    {
        auto value = std::atoi(lookahead_.value.c_str());
        eat(lookahead_.type);
        return make<type_int>(std::move(value));
    }
    else if (lookahead_.type == token_type::id)
    {
        // parse a function call
        auto id = lookahead_;
        eat(token_type::id);
        eat(token_type::left_parent);
        auto args = param_list();
        eat(token_type::right_parent);
        
        return environment_.call_var(id.value, args.begin(), args.end());
    }
    throw parse_error(
        "Expected " + to_string(token_type::left_parent) + ", " + 
        to_string(token_type::number) + " or " + 
        to_string(token_type::id) +  ", got " + 
        to_string(lookahead_) + "."); 
}

std::vector<dice::parser::value_type> dice::parser::param_list()
{
    std::vector<value_type> args;
    if (lookahead_.type == token_type::right_parent)
    {
        return args; // no arguments
    }

    for (;;)
    {
        args.emplace_back(expr());
        if (lookahead_.type != token_type::param_delim)
        {
            break;
        }
        eat(token_type::param_delim);
    }
    return args;
}

void dice::parser::eat(token_type type)
{
    if (lookahead_.type != type)
    {
        throw parse_error(
            "Expected " + to_string(type) + ", got " + to_string(lookahead_) + ".");
    }
    lookahead_ = lexer_->read_token();
}


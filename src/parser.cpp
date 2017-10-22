#include "parser.hpp"

// dice expression parser

dice::parser::parser(lexer* l, logger* log) : 
    lexer_(l),
    log_(log)
{
}

dice::parser::value_type dice::parser::parse()
{
    lookahead_ = lexer_->read_token();

    // check if it's valid to parse an expression
    while (lookahead_.type != token_type::end && !is_valid_expr())
    {
        error("Invalid token at the beginning of an expression: " + 
            to_string(lookahead_));
        // just ignore the token, we can't do much with it at this point
        eat(lookahead_.type);
    }

    // if there is nothing left, just end with 0
    if (lookahead_.type == token_type::end)
    {
        return make<type_int>(0);
    }

    auto result = expr();
    eat(token_type::end); // make sure we've processed the whole expression
    return result;
}

bool dice::parser::is_valid_expr() const
{
    return is_valid_add();
}

dice::parser::value_type dice::parser::expr()
{
    auto left = add();
    if (lookahead_.type == token_type::in)
    {
        eat(token_type::in);
        eat(token_type::left_square_bracket);

        // parse lower bound of the interval
        if (!is_valid_add())
        {
            error("Invalid operand for the lower bound of operator in");
            return left;
        }
        auto lower_bound = add();

        eat(token_type::param_delim);

        // parse upper bound of the interval
        if (!is_valid_add())
        {
            error("Invalid operand for the upper bound of operator in");
            return left;
        }
        auto upper_bound = add();

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

bool dice::parser::is_valid_add() const 
{
    return is_valid_mult();
}

dice::parser::value_type dice::parser::add()
{
    std::string op;
    auto result = mult();
    for (;;)
    {
        // decide on the next operation
        if (lookahead_.type == token_type::add)
        {
            eat(token_type::add);
            op = "+";
        }
        else if (lookahead_.type == token_type::sub)
        {
            eat(token_type::sub);
            op = "-";
        }
        else 
        {
            break;
        }

        // compute the operator if there won't be any parse error
        if (is_valid_mult())
        {
            result = environment_.call(op, std::move(result), mult());
        }
        else // otherwise, ignore the operator
        {
            error("Invalid operand for binary operator " + op);
        }
    }
    return result;
}

bool dice::parser::is_valid_mult() const 
{
    return is_valid_dice_roll();
}

dice::parser::value_type dice::parser::mult()
{
    std::string op;
    auto result = dice_roll();
    for (;;)
    {
        // decide on the next operation
        if (lookahead_.type == token_type::mult)
        {
            eat(token_type::mult);
            op = "*";
        }
        else if (lookahead_.type == token_type::div)
        {
            eat(token_type::div);
            op = "/";
        }
        else 
        {
            break;
        }

        // compute the operation if there won't be any parse error
        if (is_valid_dice_roll())
        {
            result = environment_.call(op, std::move(result), dice_roll());
        }
        else // otherwise, ignore the operator
        {
            error("Invalid operand for binary operator " + op);
        }
    }
    return result;
}

bool dice::parser::is_valid_dice_roll() const 
{
    return lookahead_.type == token_type::sub || is_valid_factor();
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

            // only use the factor production if there won't be any parse error
            if (is_valid_factor())
            {
                result = environment_.call("roll", std::move(result), factor());
            }
            else // otherwise, ignore the operator 
            {
                error("Invalid operand for binary operator D (dice roll)");
            }
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

bool dice::parser::is_valid_factor() const 
{
    return lookahead_.type == token_type::left_parent ||
        lookahead_.type == token_type::number ||
        lookahead_.type == token_type::id;
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
    
    error("Expected " + to_string(token_type::left_parent) + ", " + 
        to_string(token_type::number) + " or " + 
        to_string(token_type::id) +  ", got " + 
        to_string(lookahead_) + "."); 
    return make<type_int>(0);
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
        // skip invalid tokens at the beginning
        // until there is a valid token or 
        // some of the tokes from FOLLOW(expr)
        while (lookahead_.type != token_type::end && 
            lookahead_.type != token_type::param_delim &&
            lookahead_.type != token_type::right_parent &&
            !is_valid_expr())
        {
            error("Invalid token at the beginning of an expression: " + 
                to_string(lookahead_));
            eat(lookahead_.type);
        }

        if (is_valid_expr())
        {
            args.emplace_back(expr());
        }

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
        // report the error and "insert" the token (i.e. assume it's there) 
        error("Expected " + 
            to_string(type) + ", got " + 
            to_string(lookahead_) + ".");
    }
    lookahead_ = lexer_->read_token();
}

void dice::parser::error(const std::string& message)
{
    log_->error(
        lexer_->location().line, 
        lexer_->location().col, 
        message);
}
#include "environment.hpp"

// functions implementation

static dice::user_function::return_type dice_max(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    using fn = dice::function_traits;

    if (fn::argc(first, last) <= 0)
        throw dice::compiler_error("Function max() expects at least 1 argument.");

    auto it = first;
    auto result = fn::arg<dice::dice_rand_var>(it);
    ++it;

    for (; it != last; ++it)
    {
        result->data() = max(
            result->data(), 
            fn::arg<dice::dice_rand_var>(it)->data());
    }
    return std::move(*first);
}

static dice::user_function::return_type dice_min(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    using fn = dice::function_traits;

    if (fn::argc(first, last) <= 0)
        throw dice::compiler_error("Function min() expects at least 1 argument.");

    auto it = first;
    auto result = fn::arg<dice::dice_rand_var>(it);
    ++it;

    for (; it != last; ++it)
    {
        result->data() = min(
            result->data(), 
            fn::arg<dice::dice_rand_var>(it)->data());
    }
    return std::move(*first);
}

static dice::user_function::return_type dice_expectation(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    using fn = dice::function_traits;

    if (fn::argc(first, last) != 1)
        throw dice::compiler_error("Function expectation() expects exactly 1 argument.");

    return std::make_unique<dice::dice_double>(
        fn::arg<dice::dice_rand_var>(first)->data().expected_value());
}

static dice::user_function::return_type dice_variance(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    using fn = dice::function_traits;

    if (fn::argc(first, last) != 1)
        throw dice::compiler_error("Function variance() expects exactly 1 argument.");

    return std::make_unique<dice::dice_double>(
        fn::arg<dice::dice_rand_var>(first)->data().variance());
}

// operator functions

template<typename T>
static dice::user_function::return_type dice_add(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    using fn = dice::function_traits;

    if (fn::argc(first, last) != 2)
        throw dice::compiler_error("Binary operator + expects exactly 2 arguments.");

    auto lhs = fn::arg<T>(first);
    auto rhs = fn::arg<T>(first + 1);
    lhs->data() = lhs->data() + rhs->data();
    return std::move(*first);
}

// environment code

dice::environment::environment()
{
    functions_.insert(std::make_pair(
        "max", user_function(dice_min)
    ));
    functions_.insert(std::make_pair(
        "min", user_function(dice_max)
    ));
    functions_.insert(std::make_pair(
        "expectation", user_function(dice_expectation)
    ));
    functions_.insert(std::make_pair(
        "variance", user_function(dice_variance)
    ));
    functions_.insert(std::make_pair(
        "+", user_function(dice_add<dice_rand_var>, { 
                dice_rand_var::get_type_id(), 
                dice_rand_var::get_type_id() 
            })
    ));

    args_.resize(function_traits::max_argc);
}

dice::user_function::return_type dice::environment::call_var(
    const std::string& name,
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    // find the function in the function table
    auto it = functions_.find(name);
    if (it == functions_.end())
    {
        throw compiler_error("Unknown function " + name + "().");
    }
    auto&& function = it->second;

    // check argument counts
    auto expected_argc = function_traits::argc(first, last);
    auto actual_argc = function.arg_types().size();
    if (expected_argc != actual_argc)
    {
        throw compiler_error(
            name + "(): expects " + std::to_string(expected_argc) + 
            ", got " + std::to_string(actual_argc));
    }

    // check argument types
    auto arg_it = first;
    for (auto&& expected_type : function.arg_types())
    {
        auto actual_type = (*arg_it)->type();
        if (expected_type != actual_type)
        {
            throw compiler_error(
                name + "(): invalid argument type (argument " + 
                std::to_string(std::distance(first, arg_it)) + "). Expected " + 
                expected_type + ", got " + actual_type);
        }
        ++arg_it;
    }

    // execute it
    return function(first, last);
}
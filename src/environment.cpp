#include "environment.hpp"

// functions implementation

// operator functions

template<typename T>
dice::user_function::return_type dice_add(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    return dice::make<T>(
        fn::arg<T>(first)->data() + fn::arg<T>(first + 1)->data()
    );
}

template<typename T>
dice::user_function::return_type dice_sub(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    return dice::make<T>(
        fn::arg<T>(first)->data() - fn::arg<T>(first + 1)->data()
    );
}

template<typename T>
dice::user_function::return_type dice_mult(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    return dice::make<T>(
        fn::arg<T>(first)->data() * fn::arg<T>(first + 1)->data()
    );
}

template<typename T>
dice::user_function::return_type dice_div(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    return dice::make<T>(
        fn::arg<T>(first)->data() / fn::arg<T>(first + 1)->data()
    );
}

static dice::user_function::return_type dice_roll(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    return dice::make<dice::type_rand_var>(
        dice::type_rand_var::value_type::roll(
            fn::arg<dice::type_rand_var>(first)->data(),
            fn::arg<dice::type_rand_var>(first + 1)->data()
        )
    );
}

// environment code

dice::environment::environment()
{
    args_.resize(function_traits::max_argc);

    // user functions
    add_function("+", {
        dice_add<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("+", {
        dice_add<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("+", {
        dice_add<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("-", {
        dice_sub<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("-", {
        dice_sub<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("-", {
        dice_sub<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("*", {
        dice_mult<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("*", {
        dice_mult<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("*", {
        dice_mult<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("/", {
        dice_div<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("/", {
        dice_div<type_double>, { type_double::id(), type_double::id() }
    });
    add_function("/", {
        dice_div<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("roll", {
        dice_roll, { type_rand_var::id(), type_rand_var::id() }
    });

    // type conversions
    conversions_.add_conversion(type_int::id(), type_double::id(), [](auto&& value)
    {
        auto&& int_value = dynamic_cast<type_int&>(*value);
        return make<type_double>(static_cast<double>(int_value.data()));
    }, 1);

    conversions_.add_conversion(type_int::id(), type_rand_var::id(), [](auto&& value)
    {
        auto&& int_value = dynamic_cast<type_int&>(*value);
        return make<type_rand_var>(constant_tag{}, int_value.data());
    }, 1);
}

void dice::environment::add_function(const std::string& name, user_function&& func)
{
    auto result = functions_.insert(std::make_pair(
        name, 
        std::vector<user_function>{}
    ));
    result.first->second.push_back(std::move(func));
}

dice::user_function::return_type dice::environment::call_var(
    const std::string& name,
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator last)
{
    auto expected_argc = function_traits::argc(first, last);

    // find functions in the function table
    auto it = functions_.find(name);
    if (it == functions_.end())
    {
        throw compiler_error("Unknown function " + name + "().");
    }
    auto&& functions = it->second;

    // choose function with the lowest conversion cost
    user_function* min_func = nullptr;
    conversions::cost_type min_cost = conversions::max_cost;
    for (auto&& function : functions)
    {
        if (function.argc() != expected_argc)
        {
            continue;
        }

        // calculate conversion cost for this function
        auto it = first;
        conversions::cost_type cost = 0;
        for (auto&& to_type : function.args())
        {
            auto from_type = (*it)->type();
            auto conv_cost = conversions_.cost(from_type, to_type);
            if (conv_cost == conversions::max_cost)
            {
                cost = conv_cost;
                break;
            }
            cost += conv_cost;
            ++it;
        }

        // update minimal conversion function 
        if (cost < min_cost)
        {
            min_cost = cost;
            min_func = &function;
        }
    }

    // if there is no suitable conversion, the function call fails
    if (min_func == nullptr)
    {
        throw compiler_error("No matching function to call: " + name + "()");
    }

    // convert arguments
    std::size_t i = 0;
    for (auto it = first; it != last; ++it, ++i)
    {
        type_id from = (*it)->type();
        type_id to = min_func->args()[i];
        *it = conversions_.convert(from, to, std::move(*it));
    }

    // execute it
    return (*min_func)(first, last);
}
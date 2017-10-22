#include "environment.hpp"

// functions implementation

static dice::user_function::return_type dice_expectation(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    return make<type_double>(
        fn::arg<type_rand_var>(first)->data().expected_value()
    );
}

static dice::user_function::return_type dice_variance(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    return make<type_double>(
        fn::arg<type_rand_var>(first)->data().variance()
    );
}

// operator functions

template<typename T>
dice::user_function::return_type dice_add(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() + fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
dice::user_function::return_type dice_sub(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() - fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
dice::user_function::return_type dice_mult(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() * fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
dice::user_function::return_type dice_div(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() / fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
dice::user_function::return_type dice_unary_minus(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    fn::arg<T>(first)->data() = -fn::arg<T>(first)->data();
    return std::move(*first);
}

static dice::user_function::return_type dice_roll(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using fn = dice::function_traits;
    using namespace dice;
    fn::arg<type_rand_var>(first)->data() = type_rand_var::value_type::roll(
        fn::arg<type_rand_var>(first)->data(),
        fn::arg<type_rand_var>(first + 1)->data()
    );
    return std::move(*first);
}

static dice::user_function::return_type dice_rand_var_in(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    fn::arg<type_rand_var>(first)->data() = fn::arg<type_rand_var>(first)->data().in(
        fn::arg<type_double>(first + 1)->data(),
        fn::arg<type_double>(first + 2)->data()
    );
    return std::move(*first);
}

static dice::user_function::return_type dice_less_than(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().less_than(b->data());
    return std::move(*first);
}

static dice::user_function::return_type dice_less_than_or_equal(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().less_than_or_equal(b->data());
    return std::move(*first);
}

static dice::user_function::return_type dice_equal(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().equal(b->data());
    return std::move(*first);
}

static dice::user_function::return_type dice_not_equal(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().not_equal(b->data());
    return std::move(*first);
}

static dice::user_function::return_type dice_greater_than(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().greater_than(b->data());
    return std::move(*first);
}

static dice::user_function::return_type dice_greater_than_or_equal(
    dice::user_function::args_iterator first,
    dice::user_function::args_iterator)
{
    using namespace dice;
    using fn = function_traits;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().greater_than_or_equal(b->data());
    return std::move(*first);
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
    add_function("unary-", {
        dice_unary_minus<type_int>, { type_int::id() }
    });
    add_function("unary-", {
        dice_unary_minus<type_double>, { type_double::id() }
    });
    add_function("unary-", {
        dice_unary_minus<type_rand_var>, { type_rand_var::id() }
    });
    add_function("roll", {
        dice_roll, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("expectation", {
        dice_expectation, { type_rand_var::id() }
    });
    add_function("variance", {
        dice_variance, { type_rand_var::id() }
    });
    add_function("in", {
        dice_rand_var_in, { type_rand_var::id(), type_double::id(), type_double::id() }
    });
    add_function("<", {
        dice_less_than, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("<=", {
        dice_less_than_or_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("==", {
        dice_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("!=", {
        dice_not_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function(">=", {
        dice_greater_than_or_equal, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function(">", {
        dice_greater_than, { type_rand_var::id(), type_rand_var::id() }
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
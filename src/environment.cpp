#include "environment.hpp"

using fn = dice::function_traits;

// functions implementation

static fn::return_type dice_expectation(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    return make<type_double>(
        fn::arg<type_rand_var>(first)->data().expected_value()
    );
}

static fn::return_type dice_variance(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    return make<type_double>(
        fn::arg<type_rand_var>(first)->data().variance()
    );
}

// operator functions

template<typename T>
fn::return_type dice_add(
    fn::args_iterator first,
    fn::args_iterator)
{
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() + 
        fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
fn::return_type dice_sub(
    fn::args_iterator first,
    fn::args_iterator)
{
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() - 
        fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
fn::return_type dice_mult(
    fn::args_iterator first,
    fn::args_iterator)
{
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() * 
        fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
fn::return_type dice_div(
    fn::args_iterator first,
    fn::args_iterator)
{
    fn::arg<T>(first)->data() = fn::arg<T>(first)->data() / 
        fn::arg<T>(first + 1)->data();
    return std::move(*first);
}

template<typename T>
fn::return_type dice_unary_minus(
    fn::args_iterator first,
    fn::args_iterator)
{
    fn::arg<T>(first)->data() = -fn::arg<T>(first)->data();
    return std::move(*first);
}

static fn::return_type dice_roll_op(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    fn::arg<type_rand_var>(first)->data() = roll(
        fn::arg<type_rand_var>(first)->data(),
        fn::arg<type_rand_var>(first + 1)->data()
    );
    return std::move(*first);
}

static fn::return_type dice_rand_var_in(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto&& var = fn::arg<type_rand_var>(first)->data();
    auto&& lower_bound = fn::arg<type_double>(first + 1)->data();
    auto&& upper_bound = fn::arg<type_double>(first + 2)->data();
    
    var = var.in(lower_bound, upper_bound);
    return std::move(*first);
}

static fn::return_type dice_less_than(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().less_than(b->data());
    return std::move(*first);
}

static fn::return_type dice_less_than_or_equal(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().less_than_or_equal(b->data());
    return std::move(*first);
}

static fn::return_type dice_equal(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().equal(b->data());
    return std::move(*first);
}

static fn::return_type dice_not_equal(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().not_equal(b->data());
    return std::move(*first);
}

static fn::return_type dice_greater_than(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().greater_than(b->data());
    return std::move(*first);
}

static fn::return_type dice_greater_than_or_equal(
    fn::args_iterator first,
    fn::args_iterator)
{
    using namespace dice;
    auto a = fn::arg<type_rand_var>(first);
    auto b = fn::arg<type_rand_var>(first + 1);
    a->data() = a->data().greater_than_or_equal(b->data());
    return std::move(*first);
}

template<typename T>
static fn::return_type dice_min(fn::args_iterator first, fn::args_iterator)
{
    using namespace dice;
    using namespace std;
    auto a = fn::arg<T>(first);
    auto b = fn::arg<T>(first + 1);
    a->data() = min(a->data(), b->data());
    return std::move(*first);
}

template<typename T>
static fn::return_type dice_max(fn::args_iterator first, fn::args_iterator)
{
    using namespace dice;
    using namespace std;
    auto a = fn::arg<T>(first);
    auto b = fn::arg<T>(first + 1);
    a->data() = max(a->data(), b->data());
    return std::move(*first);
}

// generate a random number
#ifndef DISABLE_RNG

static std::random_device dev;

template<typename ProbType = double>
struct dice_roll 
{
    std::default_random_engine engine;
    std::uniform_real_distribution<ProbType> dist;

    dice_roll() : engine(dev()), dist(0, 1) {}

    fn::return_type operator()(
        fn::args_iterator first,
        fn::args_iterator)
    {
        using namespace dice;

        ProbType sum = 0;
        auto value = dist(engine);
        auto&& var = fn::arg<type_rand_var>(first)->data();
        for (auto&& pair : var.probability())
        {
            if (sum + pair.second >= value)
            {
                return make<type_int>(pair.first);
            }
            sum += pair.second;
        }
        return make<type_int>(var.probability().end()->first);
    } 
};

#endif // DISABLE_RNG

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
    add_function("__roll_op", {
        dice_roll_op, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("expectation", {
        dice_expectation, { type_rand_var::id() }
    });
    add_function("variance", {
        dice_variance, { type_rand_var::id() }
    });
    add_function("in", {
        dice_rand_var_in, { 
            type_rand_var::id(), 
            type_double::id(), 
            type_double::id() }
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
        dice_greater_than_or_equal, { 
            type_rand_var::id(), 
            type_rand_var::id() }
    });
    add_function(">", {
        dice_greater_than, { type_rand_var::id(), type_rand_var::id() }
    });

#ifndef DISABLE_RNG
    add_function("roll", {
        dice_roll<double>{}, { type_rand_var::id() }
    });
#endif // DISABLE_RNG

    // Compute minimum of 2 values
    add_function("min", {
        dice_min<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("min", {
        dice_min<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("min", {
        dice_min<type_double>, { type_double::id(), type_double::id() }
    });
    
    // Compute maximum of 2 values
    add_function("max", {
        dice_max<type_rand_var>, { type_rand_var::id(), type_rand_var::id() }
    });
    add_function("max", {
        dice_max<type_int>, { type_int::id(), type_int::id() }
    });
    add_function("max", {
        dice_max<type_double>, { type_double::id(), type_double::id() }
    });
}

void dice::environment::add_function(
    const std::string& name, 
    user_function&& func)
{
    auto result = functions_.insert(std::make_pair(
        name, 
        std::vector<user_function>{}
    ));
    result.first->second.push_back(std::move(func));
}

void dice::environment::set_var(const std::string& name, value_type&& value)
{
    auto it = variables_.find(name);
    if (it != variables_.end())
    {
        it->second = std::move(value);
    }
    else 
    {
        variables_.insert(std::make_pair(
            name,
            std::move(value)
        ));
    }
}

dice::base_value* dice::environment::get_var(const std::string& name)
{
    auto it = variables_.find(name);
    if (it == variables_.end())
        return nullptr;
    return it->second.get();
}

const dice::base_value* dice::environment::get_var(const std::string& name) const
{
    auto it = variables_.find(name);
    if (it == variables_.end())
        return nullptr;
    return it->second.get();
}

fn::return_type dice::environment::call_var(
    const std::string& name,
    fn::args_iterator first,
    fn::args_iterator last)
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
#include "catch.hpp"
#include "environment.hpp"

using freq_list = dice::random_variable<
    dice::storage::int_type, 
    dice::storage::double_type>::frequency_list;

TEST_CASE("Call operator + on integers", "[environment]")
{
    dice::environment env;
    auto result = env.call("+", 
        dice::make<dice::type_int>(1), 
        dice::make<dice::type_int>(2));
    REQUIRE(result->type() == dice::type_int::id());

    auto int_result = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE((int_result->data() == 3));
}

TEST_CASE("Call operator + on random variables", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_rand_var>(freq_list{
        std::make_pair(1, 2),
        std::make_pair(2, 3)
    });
    auto b = dice::make<dice::type_rand_var>(freq_list{
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    });

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto var = rand_var_result->data().to_random_variable();
    REQUIRE(var.probability(3) == Approx(1 / 5.0));
    REQUIRE(var.probability(4) == Approx(1 / 2.0));
    REQUIRE(var.probability(5) == Approx(3 / 10.0));
}

TEST_CASE("Operator + converts an int to a random variable if one argument is a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_int>(1);
    auto b = dice::make<dice::type_rand_var>(freq_list{
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    });

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto var = rand_var_result->data().to_random_variable();
    REQUIRE(var.probability(3) == Approx(1 / 2.0));
    REQUIRE(var.probability(4) == Approx(1 / 2.0));
}

TEST_CASE("Roll operator converts int arguments to a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_int>(1);
    auto b = dice::make<dice::type_int>(6);

    auto result = env.call("roll_op", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto var = rand_var_result->data().to_random_variable();
    REQUIRE(var.probability(1) == Approx(1 / 6.0));
    REQUIRE(var.probability(2) == Approx(1 / 6.0));
    REQUIRE(var.probability(3) == Approx(1 / 6.0));
    REQUIRE(var.probability(4) == Approx(1 / 6.0));
    REQUIRE(var.probability(5) == Approx(1 / 6.0));
    REQUIRE(var.probability(6) == Approx(1 / 6.0));
}

TEST_CASE("Call function using vector as arguments", "[environment]")
{
    std::vector<std::unique_ptr<dice::base_value>> args;
    args.push_back(dice::make<dice::type_int>(3));
    args.push_back(dice::make<dice::type_int>(5));

    dice::environment env;
    auto result = env.call_var("+", args.begin(), args.end());
    REQUIRE(result->type() == dice::type_int::id());

    auto int_result = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE((int_result->data() == 8));
}

TEST_CASE("Call unary - function on int", "[environment]")
{
    dice::environment env;
    auto result = env.call("unary-", dice::make<dice::type_int>(5));
    REQUIRE(result->type() == dice::type_int::id());

    auto int_type = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE((int_type->data() == -5));
}

TEST_CASE("Call unary - function on double", "[environment]")
{
    dice::environment env;
    auto result = env.call("unary-", dice::make<dice::type_double>(3.14));
    REQUIRE(result->type() == dice::type_double::id());

    auto int_type = dynamic_cast<dice::type_double*>(result.get());
    REQUIRE(int_type->data() == -3.14);
}

TEST_CASE("Call unary - function on random variable", "[environment]")
{
    dice::environment env;
    auto value = dice::make<dice::type_rand_var>(dice::bernoulli_tag{}, 0.4);
    auto result = env.call("unary-", std::move(value));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto drv = dynamic_cast<dice::type_rand_var*>(result.get());
    auto var = drv->data().to_random_variable();
    REQUIRE(var.probability(-1) == 0.4);
    REQUIRE(var.probability(0) == 0.6);
}

TEST_CASE("Call the variance function on a random variable", "[environment]")
{
    dice::environment env;
    auto value = dice::make<dice::type_rand_var>(dice::bernoulli_tag{}, 0.4);
    auto result = env.call("variance", std::move(value));
    REQUIRE(result->type() == dice::type_double::id());

    auto double_type = dynamic_cast<dice::type_double*>(result.get());
    REQUIRE(double_type->data() == Approx(0.4 * 0.6));
}

TEST_CASE("Call the expectation function on a random variable", "[environment]")
{
    dice::environment env;
    auto value = dice::make<dice::type_rand_var>(dice::bernoulli_tag{}, 0.4);
    auto result = env.call("expectation", std::move(value));
    REQUIRE(result->type() == dice::type_double::id());

    auto double_type = dynamic_cast<dice::type_double*>(result.get());
    REQUIRE(double_type->data() == Approx(0.4));
}

TEST_CASE("Call in operator on a random variable and int interval", "[environment]")
{
    dice::environment env;
    auto input = dice::make<dice::type_rand_var>(freq_list{
        std::make_pair(1, 1),
        std::make_pair(2, 1),
        std::make_pair(3, 1),
        std::make_pair(4, 1),
        std::make_pair(5, 1)
    });
    auto lower = dice::make<dice::type_int>(2);
    auto upper = dice::make<dice::type_int>(4);

    auto result = env.call("in", std::move(input), std::move(lower), std::move(upper));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto var = dynamic_cast<dice::type_rand_var*>(result.get())->data().to_random_variable();
    REQUIRE(var.probability(1) == Approx(3 / 5.0));
    REQUIRE(var.probability(0) == Approx(2 / 5.0));
}

#ifndef DISABLE_RNG

TEST_CASE("Generate a random number from given distribution", "[environment]")
{
    dice::environment env;

    // 15 (14.7 exactly) is the expected number of iterations to generate all numbers
    for (int i = 0; i < 15; i++) 
    {
        auto var = dice::make<dice::type_rand_var>(freq_list{
            std::make_pair(1, 1),
            std::make_pair(2, 1),
            std::make_pair(3, 1),
            std::make_pair(4, 1),
            std::make_pair(5, 1),
            std::make_pair(6, 1)
        });
        auto result = env.call("roll", std::move(var));
        REQUIRE(result->type() == dice::type_int::id());
        auto&& value = dynamic_cast<dice::type_int&>(*result).data();
        REQUIRE((value >= 1 && value <= 6));
    }
}

#endif // DISABLE_RNG

TEST_CASE("Set value of unknown variable", "[environment]")
{
    dice::environment env;

    env.set_var("test", dice::make<dice::type_int>(14));

    auto value = env.get_var("test");
    REQUIRE((dynamic_cast<dice::type_int*>(value)->data() == 14));
}

TEST_CASE("Overwrite value of set variable", "[environment]")
{
    dice::environment env;

    env.set_var("test", dice::make<dice::type_int>(14));
    env.set_var("test", dice::make<dice::type_int>(41));

    auto value = env.get_var("test");
    REQUIRE((dynamic_cast<dice::type_int*>(value)->data() == 41));
}

TEST_CASE("Get unknown variable", "[environment]")
{
    dice::environment env;

    env.set_var("test", dice::make<dice::type_int>(1));

    REQUIRE(env.get_var("tesT") == nullptr);
    REQUIRE(env.get_var("unknown") == nullptr);
}
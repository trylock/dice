#include "catch.hpp"
#include "environment.hpp"

TEST_CASE("Call operator + on integers", "[environment]")
{
    dice::environment env;
    auto result = env.call("+", 
        dice::make<dice::type_int>(1), 
        dice::make<dice::type_int>(2));
    REQUIRE(result->type() == dice::type_int::id());

    auto int_result = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE(int_result->data() == 3);
}

TEST_CASE("Call operator + on random variables", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_rand_var>(
        std::make_pair(1, 2),
        std::make_pair(2, 3)
    );
    auto b = dice::make<dice::type_rand_var>(
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    );

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto&& prob = rand_var_result->data().probability();
    REQUIRE(prob.find(3)->second == Approx(1 / 5.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(5)->second == Approx(3 / 10.0));
}

TEST_CASE("Operator + converts an int to a random variable if one argument is a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_int>(1);
    auto b = dice::make<dice::type_rand_var>(
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    );

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto&& prob = rand_var_result->data().probability();
    REQUIRE(prob.find(3)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 2.0));
}

TEST_CASE("Roll operators converts int arguments for the roll operator to a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::type_int>(1);
    auto b = dice::make<dice::type_int>(6);

    auto result = env.call("roll", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::type_rand_var::id());

    auto rand_var_result = dynamic_cast<dice::type_rand_var*>(result.get());
    auto&& prob = rand_var_result->data().probability();
    REQUIRE(prob.find(1)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(2)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(3)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(5)->second == Approx(1 / 6.0));
    REQUIRE(prob.find(6)->second == Approx(1 / 6.0));
}

TEST_CASE("Call function using iterators as arguments", "[environment]")
{
    std::vector<std::unique_ptr<dice::base_value>> args;
    args.push_back(dice::make<dice::type_int>(3));
    args.push_back(dice::make<dice::type_int>(5));

    dice::environment env;
    auto result = env.call_var("+", args.begin(), args.end());
    REQUIRE(result->type() == dice::type_int::id());

    auto int_result = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE(int_result->data() == 8);
}

TEST_CASE("Call unary - function on int", "[environment]")
{
    dice::environment env;
    auto result = env.call("unary-", dice::make<dice::type_int>(5));
    REQUIRE(result->type() == dice::type_int::id());

    auto int_type = dynamic_cast<dice::type_int*>(result.get());
    REQUIRE(int_type->data() == -5);
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
    auto&& prob = drv->data().probability();
    REQUIRE(prob.find(-1)->second == 0.4);
    REQUIRE(prob.find(0)->second == 0.6);
}
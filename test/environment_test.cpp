#include "catch.hpp"
#include "environment.hpp"

TEST_CASE("Call operator + on integers", "[environment]")
{
    dice::environment env;
    auto result = env.call("+", 
        dice::make<dice::dice_int>(1), 
        dice::make<dice::dice_int>(2));
    REQUIRE(result->type() == dice::dice_int::get_type_id());

    auto int_result = dynamic_cast<dice::dice_int*>(result.get());
    REQUIRE(int_result->data() == 3);
}

TEST_CASE("Call operator + on random variables", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::dice_rand_var>(
        std::make_pair(1, 2),
        std::make_pair(2, 3)
    );
    auto b = dice::make<dice::dice_rand_var>(
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    );

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::dice_rand_var::get_type_id());

    auto rand_var_result = dynamic_cast<dice::dice_rand_var*>(result.get());
    auto&& prob = rand_var_result->data().probability();
    REQUIRE(prob.find(3)->second == Approx(1 / 5.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(5)->second == Approx(3 / 10.0));
}

TEST_CASE("Operator + converts an int to a random variable if one argument is a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::dice_int>(1);
    auto b = dice::make<dice::dice_rand_var>(
        std::make_pair(2, 1),
        std::make_pair(3, 1)
    );

    auto result = env.call("+", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::dice_rand_var::get_type_id());

    auto rand_var_result = dynamic_cast<dice::dice_rand_var*>(result.get());
    auto&& prob = rand_var_result->data().probability();
    REQUIRE(prob.find(3)->second == Approx(1 / 2.0));
    REQUIRE(prob.find(4)->second == Approx(1 / 2.0));
}

TEST_CASE("Roll operators converts int arguments for the roll operator to a random variable", "[environment]")
{
    dice::environment env;
    auto a = dice::make<dice::dice_int>(1);
    auto b = dice::make<dice::dice_int>(6);

    auto result = env.call("roll", std::move(a), std::move(b));
    REQUIRE(result->type() == dice::dice_rand_var::get_type_id());

    auto rand_var_result = dynamic_cast<dice::dice_rand_var*>(result.get());
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
    args.push_back(dice::make<dice::dice_int>(3));
    args.push_back(dice::make<dice::dice_int>(5));

    dice::environment env;
    auto result = env.call_var("+", args.begin(), args.end());
    REQUIRE(result->type() == dice::dice_int::get_type_id());

    auto int_result = dynamic_cast<dice::dice_int*>(result.get());
    REQUIRE(int_result->data() == 8);
}
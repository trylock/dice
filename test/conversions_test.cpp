#include "catch.hpp"
#include "conversions.hpp"

TEST_CASE("Compute cost of conversion of the same type", "[conversions]")
{
    using namespace dice;

    conversions conv;
    REQUIRE(conv.cost(type_id::integer, type_id::integer) == 0);
    REQUIRE(conv.cost(type_id::floating_point, type_id::floating_point) == 0);
    REQUIRE(conv.cost(type_id::random_variable, type_id::random_variable) == 0);
}

TEST_CASE("Compute cost of all conversions", "[conversions]")
{
    using namespace dice;

    conversions conv;
    REQUIRE(conv.cost(type_id::integer, type_id::floating_point) == 1);
    REQUIRE(conv.cost(type_id::integer, type_id::random_variable) == 1);

    REQUIRE(conv.cost(type_id::floating_point, type_id::integer) == conversions::max_cost);
    REQUIRE(conv.cost(type_id::floating_point, type_id::random_variable) == conversions::max_cost);

    REQUIRE(conv.cost(type_id::random_variable, type_id::integer) == conversions::max_cost);
    REQUIRE(conv.cost(type_id::random_variable, type_id::floating_point) == conversions::max_cost);
}

TEST_CASE("Convert value to all other values", "[conversions]")
{
    using namespace dice;

    conversions conv;

    // int -> double
    auto int_value = make<type_int>(1);
    auto double_value = conv.convert(type_id::floating_point, std::move(int_value));
    REQUIRE(dynamic_cast<type_double&>(*double_value).data() == 1);

    // int -> random variable
    int_value = make<type_int>(1);
    auto var_value = conv.convert(type_id::random_variable, std::move(int_value));
    auto var = dynamic_cast<type_rand_var*>(var_value.get());
    auto prob = var->data().to_random_variable().probability(1);
    REQUIRE(prob == Approx(1));

    // double -> int
    double_value = make<type_double>(1.5);
    auto result = conv.convert(type_id::integer, std::move(double_value));
    REQUIRE(result == nullptr);
    
    // double -> random variable
    double_value = make<type_double>(1.5);
    result = conv.convert(type_id::random_variable, std::move(double_value));
    REQUIRE(result == nullptr);

    // random variable -> int
    var_value = make<type_rand_var>(constant_tag{}, 42);
    result = conv.convert(type_id::integer, std::move(var_value));
    REQUIRE(result == nullptr);

    // random variable -> double
    var_value = make<type_rand_var>(constant_tag{}, 42);
    result = conv.convert(type_id::floating_point, std::move(var_value));
    REQUIRE(result == nullptr);
}

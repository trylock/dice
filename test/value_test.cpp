#include <catch.hpp>
#include "value.hpp"

TEST_CASE("Compare 2 different values of the same type", "[value]")
{
    dice::type_int value_a{ 4 };
    dice::type_int value_b{ 6 };
    REQUIRE_FALSE(value_a == value_b);
    REQUIRE_FALSE(value_b == value_a);
    REQUIRE(value_a != value_b);
    REQUIRE(value_b != value_a);
}

TEST_CASE("Compare 2 same values of the same type", "[value]")
{
    dice::type_int value_a{ 6 };
    dice::type_int value_b{ 6 };
    REQUIRE(value_a == value_b);
    REQUIRE(value_b == value_a);
    REQUIRE_FALSE(value_a != value_b);
    REQUIRE_FALSE(value_b != value_a);
}

TEST_CASE("Compare 2 values of different types", "[value]")
{
    dice::type_int value_a{ 6 };
    dice::type_double value_b{ 6.0 };
    REQUIRE_FALSE(value_a == value_b);
    REQUIRE_FALSE(value_b == value_a);
    REQUIRE(value_a != value_b);
    REQUIRE(value_b != value_a);
}

class visitor_mock : public dice::value_visitor
{
public:
    int sum = 0;

    void visit(dice::type_int* value) override
    {
        sum += value->data();
    }

    void visit(dice::type_double* value) override
    {
        sum += static_cast<int>(value->data());
    }

    void visit(dice::type_rand_var* value) override
    {
        sum += value->data().begin()->first;
    }
};

TEST_CASE("Use correct visitor method", "[value]")
{
    visitor_mock visitor;

    dice::type_int value_int{ 2 };
    dice::type_double value_double{ 3.0 };
    dice::type_rand_var value_rand_var(dice::storage::random_variable_type(
         dice::constant_tag{}, 5 
    ));
    value_int.accept(&visitor);
    value_double.accept(&visitor);
    value_rand_var.accept(&visitor);
    REQUIRE(visitor.sum == 10);
}

TEST_CASE("Convert type_id to string", "[type]")
{
    REQUIRE(dice::to_string(dice::type_id::integer) == "int");
    REQUIRE(dice::to_string(dice::type_id::floating_point) == "double");
    REQUIRE(dice::to_string(dice::type_id::random_variable) == "random_variable");
}
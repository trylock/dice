#include <catch.hpp>

#include "checked.hpp"

TEST_CASE("Add integers in range", "[checked]")
{
    using namespace dice;

    REQUIRE(checked<int>{ 5 } + 7 == 12);
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } + (-4) == std::numeric_limits<int>::max() - 4);
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } + 0 == std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ std::numeric_limits<int>::min() } + 4 == std::numeric_limits<int>::min() + 4);
    REQUIRE(checked<int>{ std::numeric_limits<int>::min() } + 0 == std::numeric_limits<int>::min());
}

TEST_CASE("Handle overflow in addition", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() } + 1, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ 1 } + std::numeric_limits<int>::max(), std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() / 2 } + std::numeric_limits<int>::max() / 2 + 2, std::overflow_error);
}

TEST_CASE("Handle underflow in addition", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::min() } + (-1), std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ -1 } + std::numeric_limits<int>::min(), std::underflow_error);
}

TEST_CASE("Add unsigned integers in range", "[checked]")
{
    using namespace dice;

    REQUIRE(checked<std::uint32_t>{ 5 } + std::uint32_t{ 7 } == 12);
    REQUIRE(checked<std::uint32_t>{ std::numeric_limits<std::uint32_t>::max() } + std::uint32_t{ 0 } == std::numeric_limits<std::uint32_t>::max());
}

TEST_CASE("Handle overflown in addition for unsigned integers", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(checked<std::uint32_t>{ std::numeric_limits<std::uint32_t>::max() } + std::uint32_t{ 1 }, std::overflow_error);
    REQUIRE_THROWS_AS(checked<std::uint32_t>{ std::numeric_limits<std::uint32_t>::max() / 2 } + std::numeric_limits<std::uint32_t>::max() / 2 + std::uint32_t{ 2 }, std::overflow_error);
}

TEST_CASE("Subtract integers in range", "[checked]")
{
    using namespace dice;

    REQUIRE(checked<int>{ 5 } - 7 == -2);
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } - 4 == std::numeric_limits<int>::max() - 4);
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } - 0 == std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ std::numeric_limits<int>::min() } - (-4) == std::numeric_limits<int>::min() + 4);
    REQUIRE(checked<int>{ std::numeric_limits<int>::min() } - 0 == std::numeric_limits<int>::min());
}

TEST_CASE("Handle overflow in subtraction", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() } - (-1), std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() / 2 } - (-(std::numeric_limits<int>::max() / 2)) - (-2), std::overflow_error);
}

TEST_CASE("Handle underflow in subtraction", "[checked]")
{
    using namespace dice;

    auto half_max = std::numeric_limits<int>::max() / 2;
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::min() } - 1, std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ -half_max } - half_max - half_max, std::underflow_error);
}

TEST_CASE("Multiply integers in range", "[checked]")
{
    using namespace dice;

    REQUIRE(checked<int>{ 5 } * 10 == 50);
    REQUIRE(checked<int>{ -1 } * -1 == 1);
    REQUIRE(checked<int>{ 1 } * -1 == -1);
    REQUIRE(checked<int>{ -1 } * 1 == -1);
    REQUIRE(checked<int>{ 1 } * 1 == 1);
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } * -1 == -std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ -1 } * std::numeric_limits<int>::max() == -std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } * 0 == 0);
    REQUIRE(checked<int>{ 0 } * std::numeric_limits<int>::max() == 0);
}

TEST_CASE("Handle overflow in multiplication", "[checked]")
{
    using namespace dice;

    auto half_max = std::numeric_limits<int>::max() / 2;
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::min() } * -1, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ -1 } * std::numeric_limits<int>::min(), std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() } * 2, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ -half_max } * -half_max, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ half_max } * half_max, std::overflow_error);
}

TEST_CASE("Handle underflow in multiplication", "[checked]")
{
    using namespace dice;

    auto half_min = std::numeric_limits<int>::min() / 2;
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::min() } * 2, std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ 2 } * std::numeric_limits<int>::min(), std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() } * -2, std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ -2 } * std::numeric_limits<int>::max(), std::underflow_error);
    REQUIRE_THROWS_AS(checked<int>{ half_min } * -half_min, std::underflow_error);
}

TEST_CASE("Divide integers in range", "[checked]")
{
    using namespace dice;

    REQUIRE(checked<int>{ -1 } / std::numeric_limits<int>::min() == 0);
    REQUIRE(checked<int>{ std::numeric_limits<int>::min() } / 1 == std::numeric_limits<int>::min());
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } / 1 == std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } / -1 == -std::numeric_limits<int>::max());
    REQUIRE(checked<int>{ std::numeric_limits<int>::max() } / 2 == std::numeric_limits<int>::max() / 2);
    REQUIRE(checked<int>{ 0 } / std::numeric_limits<int>::min() == 0);
    REQUIRE(checked<int>{ 0 } / std::numeric_limits<int>::max() == 0);
}

TEST_CASE("Handle division by zero", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(checked<int>{ 0 } / 0, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ 1 } / 0, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::max() } / 0, std::overflow_error);
    REQUIRE_THROWS_AS(checked<int>{ std::numeric_limits<int>::min() } / 0, std::overflow_error);
}

TEST_CASE("Compute unary minus of a value in range", "[checked]")
{
    using namespace dice;

    REQUIRE(-checked<int>{ 5 } == -5);
    REQUIRE(-checked<int>{ 0 } == 0);
    REQUIRE(-checked<int>{ -5 } == 5);
    REQUIRE(-checked<int>{ std::numeric_limits<int>::max() } == -std::numeric_limits<int>::max());
}

TEST_CASE("Hnalde overflow in unary minus", "[checked]")
{
    using namespace dice;

    REQUIRE_THROWS_AS(-checked<int>{ std::numeric_limits<int>::min() }, std::overflow_error);
}
#include <catch.hpp>
#include "utils.hpp"

TEST_CASE("Compute sorted union of empty sets", "[utils]")
{
    auto result = dice::sorted_union(std::vector<int>{}, std::vector<int>{});
    REQUIRE(result.empty());
}

TEST_CASE("Compute sorted union of 1 element sets", "[utils]")
{
    std::vector<int> first{ 2 };
    std::vector<int> second{ 1 };
    auto result = dice::sorted_union(first, second);
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 2);
}

TEST_CASE("Compute sorted union of 2 non-empty sets", "[utils]")
{
    std::vector<int> first{ 1, 4, 5 };
    std::vector<int> second{ 2, 3, 6 };
    auto result = dice::sorted_union(first, second);
    REQUIRE(result.size() == 6);
    REQUIRE(result[0] == 1);
    REQUIRE(result[1] == 2);
    REQUIRE(result[2] == 3);
    REQUIRE(result[3] == 4);
    REQUIRE(result[4] == 5);
    REQUIRE(result[5] == 6);
}

TEST_CASE("Clamp floating point nubmer to the [0, 1] range", "[utils]")
{
    REQUIRE(dice::clamp(-1e20, 0.0, 1.0) == 0.0);
    REQUIRE(dice::clamp(-1.0, 0.0, 1.0) == 0.0);
    REQUIRE(dice::clamp(0.0, 0.0, 1.0) == 0.0);
    REQUIRE(dice::clamp(0.1, 0.0, 1.0) == 0.1);
    REQUIRE(dice::clamp(0.5, 0.0, 1.0) == 0.5);
    REQUIRE(dice::clamp(0.9, 0.0, 1.0) == 0.9);
    REQUIRE(dice::clamp(1.0, 0.0, 1.0) == 1.0);
    REQUIRE(dice::clamp(1.1, 0.0, 1.0) == 1.0);
    REQUIRE(dice::clamp(1e20, 0.0, 1.0) == 1.0);
}
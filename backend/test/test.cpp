#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "lib.h"

TEST_CASE("test")
{
    int result = foo();
    REQUIRE(result == 42);
}
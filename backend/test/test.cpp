#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "lib.h"

TEST_CASE("test")
{
    AdjacencyList al;
    al.addEdge(1, 2);
    al.addEdge(2, 3);
    al.addEdge(3, 4);
    al.addEdge(4, 5);
    al.addEdge(5, 6);
    al.addEdge(6, 7);
    al.addEdge(7, 8);
    al.addEdge(8, 9);
    al.addEdge(9, 10);

    std::vector<int> path = al.findPathBFS(1, 10);

    REQUIRE(path.size() == 10);
}
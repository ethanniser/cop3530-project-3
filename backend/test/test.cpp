#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "lib.h"
#include <filesystem>
#include <fstream>
#include <iostream>

void createTestData()
{
    std::filesystem::create_directory("test_data");

    // Create test edges file
    std::ofstream edgesFile("test_data/test.edges");
    edgesFile << "1 2\n";
    edgesFile << "2 3\n";
    edgesFile << "3 4\n";
    edgesFile << "4 5\n";
    edgesFile << "5 6\n";
    edgesFile << "6 7\n";
    edgesFile << "7 8\n";
    edgesFile << "8 9\n";
    edgesFile << "9 10\n";
    edgesFile.close();

    // Create test feature names file
    std::ofstream featnamesFile("test_data/test.featnames");
    featnamesFile << "0 feature1\n";
    featnamesFile << "1 feature2\n";
    featnamesFile << "2 feature3\n";
    featnamesFile.close();

    // Create test features file
    std::ofstream featFile("test_data/test.feat");
    featFile << "1 1 0 0\n"; // Node 1 has feature1
    featFile << "2 0 1 0\n"; // Node 2 has feature2
    featFile << "3 0 0 1\n"; // Node 3 has feature3
    featFile << "4 1 1 0\n"; // Node 4 has feature1 and feature2
    featFile.close();
}

void cleanupTestData()
{
    std::filesystem::remove_all("test_data");
}

TEST_CASE("Loading edges from directory")
{
    createTestData();

    SECTION("Load edges from test directory")
    {
        auto al = AdjacencyList::loadEdgesFromDirectory("test_data");

        REQUIRE(al.findPathBFS(1, 2).finalPath.size() == 2);
        REQUIRE(al.findPathBFS(1, 3).finalPath.size() == 3);
        REQUIRE(al.findPathBFS(1, 10).finalPath.size() == 10);
    }

    cleanupTestData();
}

TEST_CASE("Loading features from directory")
{
    createTestData();

    SECTION("Load features from test directory")
    {
        auto fs = FeaturesStore::loadFeaturesFromDirectory("test_data");

        REQUIRE(fs.getFeatures(1).size() == 1);
        REQUIRE(fs.getFeatures(4).size() == 2);

        auto shared = fs.sharedFeatures(1, 4);
        REQUIRE(shared.size() == 1);
        REQUIRE(shared.find("feature1") != shared.end());
    }

    cleanupTestData();
}

TEST_CASE("BFS pathfinding")
{
    createTestData();
    auto al = AdjacencyList::loadEdgesFromDirectory("test_data");

    SECTION("Direct path")
    {
        auto result = al.findPathBFS(1, 2);
        REQUIRE(result.finalPath.size() == 2);
        REQUIRE(result.finalPath[0] == 1);
        REQUIRE(result.finalPath[1] == 2);
        REQUIRE(result.parents.at(1) == -1); // Source has no parent
        REQUIRE(result.parents.at(2) == 1);
    }

    SECTION("Longer path")
    {
        auto result = al.findPathBFS(1, 5);
        REQUIRE(result.finalPath.size() == 5);
        REQUIRE(result.finalPath[0] == 1);
        REQUIRE(result.finalPath[4] == 5);
        REQUIRE(result.parents.at(1) == -1);
        REQUIRE(result.parents.at(2) == 1);
        REQUIRE(result.parents.at(3) == 2);
        REQUIRE(result.parents.at(4) == 3);
        REQUIRE(result.parents.at(5) == 4);
    }

    SECTION("No path")
    {
        auto result = al.findPathBFS(1, 100);
        REQUIRE(result.finalPath.empty());
        REQUIRE(!result.parents.empty());    // Should still contain explored nodes
        REQUIRE(result.parents.at(1) == -1); // Source parent is still -1
    }

    cleanupTestData();
}

TEST_CASE("A* pathfinding")
{
    createTestData();
    auto al = AdjacencyList::loadEdgesFromDirectory("test_data");
    auto fs = FeaturesStore::loadFeaturesFromDirectory("test_data");

    SECTION("Direct path")
    {
        auto result = al.findPathAStar(1, 2, fs);
        REQUIRE(result.finalPath.size() == 2);
        REQUIRE(result.finalPath[0] == 1);
        REQUIRE(result.finalPath[1] == 2);
    }

    SECTION("Path with shared features")
    {
        auto result = al.findPathAStar(1, 4, fs);
        REQUIRE(result.finalPath.size() == 4);
        REQUIRE(result.finalPath[0] == 1);
        REQUIRE(result.finalPath[3] == 4);
    }

    SECTION("No path")
    {
        auto result = al.findPathAStar(1, 100, fs);
        REQUIRE(result.finalPath.empty());
    }

    cleanupTestData();
}

TEST_CASE("Path exploration")
{
    createTestData();
    auto al = AdjacencyList::loadEdgesFromDirectory("test_data");

    SECTION("BFS exploration order")
    {
        auto result = al.findPathBFS(1, 3);
        REQUIRE(result.exploredPath.size() >= 3);
        REQUIRE(result.exploredPath[0] == 1);
        REQUIRE(result.exploredPath[1] == 2);
        REQUIRE(result.exploredPath[2] == 3);
    }

    SECTION("A* exploration order")
    {
        auto fs = FeaturesStore::loadFeaturesFromDirectory("test_data");
        auto result = al.findPathAStar(1, 4, fs);
        REQUIRE(result.exploredPath.size() >= 4);
        REQUIRE(result.exploredPath[0] == 1);
    }

    cleanupTestData();
}

TEST_CASE("A* heuristic scores")
{
    createTestData();
    auto al = AdjacencyList::loadEdgesFromDirectory("test_data");
    auto fs = FeaturesStore::loadFeaturesFromDirectory("test_data");

    SECTION("F-scores are computed")
    {
        auto result = al.findPathAStar(1, 4, fs);

        // Check that f-scores exist for all explored nodes
        for (const auto &node : result.exploredPath)
        {
            REQUIRE(result.heuristic_scores.find(node) != result.heuristic_scores.end());
        }

        // Node 4 (destination) has 2 features total
        // Node 1 has 1 shared feature, so heuristic = 2 - 1 = 1
        // g=0 (start node), h=1, so f=1
        REQUIRE(result.heuristic_scores.at(1) == 1);

        // For destination: g=3 (path length), h=0 (all features shared), so f=3
        REQUIRE(result.heuristic_scores.at(4) == 3);
    }

    cleanupTestData();
}

TEST_CASE("A* shared features with source")
{
    createTestData();
    auto al = AdjacencyList::loadEdgesFromDirectory("test_data");
    auto fs = FeaturesStore::loadFeaturesFromDirectory("test_data");

    SECTION("Shared features with source are computed")
    {
        auto result = al.findPathAStar(1, 4, fs);

        // Check that shared features exist for all explored nodes
        for (const auto &node : result.exploredPath)
        {
            REQUIRE(result.shared_with_source.find(node) != result.shared_with_source.end());
        }

        // Source node (1) should share all its features with itself (1 feature)
        REQUIRE(result.shared_with_source.at(1) == 1);

        // Node 4 shares feature1 with source node 1
        REQUIRE(result.shared_with_source.at(4) == 1);
    }

    cleanupTestData();
}

TEST_CASE("Graph statistics")
{
    createTestData();

    SECTION("Node and edge counts")
    {
        auto al = AdjacencyList::loadEdgesFromDirectory("test_data");

        // We have 10 nodes (1-10) in our test data
        REQUIRE(al.getNodeCount() == 10);

        // We have 9 edges in our test data (1-2, 2-3, ..., 9-10)
        // Each edge is counted once despite being bidirectional
        REQUIRE(al.getEdgeCount() == 9);
    }

    cleanupTestData();
}
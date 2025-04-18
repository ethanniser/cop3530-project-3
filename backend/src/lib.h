#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

using NodeID = int;

struct PathResult
{
    std::vector<NodeID> finalPath;
    std::vector<NodeID> exploredPath;
    std::unordered_map<NodeID, NodeID> parents;            // Maps node -> parent
    std::unordered_map<NodeID, double> heuristic_scores;   // Maps node -> heuristic score
    std::unordered_map<NodeID, size_t> shared_with_source; // Maps node -> number of features shared with source
};

class FeaturesStore
{
public:
    static FeaturesStore loadFeaturesFromDirectory(std::string path);
    static FeaturesStore loadFromCache(std::string path);
    void saveToCache(std::string path);

    void addFeature(NodeID node, std::string feature);
    std::unordered_set<std::string> getFeatures(NodeID node);
    std::unordered_set<std::string> sharedFeatures(NodeID node1, NodeID node2);

private:
    std::unordered_map<NodeID, std::unordered_set<std::string>> features;
};

class AdjacencyList
{
public:
    static AdjacencyList loadEdgesFromDirectory(std::string path);

    void addEdge(NodeID from, NodeID to);
    PathResult findPathBFS(NodeID source, NodeID destination);
    PathResult findPathAStar(NodeID source, NodeID destination, FeaturesStore &features);

    std::unordered_map<NodeID, std::unordered_set<NodeID>> getMap();
    size_t getNodeCount() const;
    size_t getEdgeCount() const;

private:
    std::unordered_map<NodeID, std::unordered_set<NodeID>> map;
};

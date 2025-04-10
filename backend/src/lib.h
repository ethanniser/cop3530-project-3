#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using NodeID = int;

struct PathResult
{
    std::vector<NodeID> finalPath;
    std::vector<NodeID> exploredPath;
};

class FeaturesStore
{
public:
    static FeaturesStore loadFeaturesFromDirectory(std::string path);

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

private:
    std::unordered_map<NodeID, std::unordered_set<NodeID>> map;
};

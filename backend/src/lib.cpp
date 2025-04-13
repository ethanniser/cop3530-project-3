#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <stack>
#include <set>
#include "lib.h"

void AdjacencyList::addEdge(NodeID from, NodeID to)
{
    // append if exists or initialize for the first time
    if (map.count(from))
    {
        map[from].insert(to);
    }
    else
    {
        map[from] = {to};
    }

    // ensures to is present as key of map if never seen before
    if (!map.count(to))
    {
        map[to] = {};
    }
}

PathResult AdjacencyList::findPathBFS(NodeID source, NodeID destination)
{
    PathResult result;
    std::unordered_map<NodeID, NodeID> parent;
    std::queue<NodeID> queue;
    std::unordered_set<NodeID> visited;

    queue.push(source);
    visited.insert(source);
    parent[source] = -1; // Mark source as having no parent

    while (!queue.empty())
    {
        NodeID current = queue.front();
        queue.pop();
        result.exploredPath.push_back(current);

        if (current == destination)
        {
            // Reconstruct path
            NodeID node = destination;
            while (node != -1)
            {
                result.finalPath.push_back(node);
                node = parent[node];
            }
            std::reverse(result.finalPath.begin(), result.finalPath.end());

            // Store parent information
            result.parents = parent;
            return result;
        }

        for (NodeID neighbor : map[current])
        {
            if (visited.find(neighbor) == visited.end())
            {
                visited.insert(neighbor);
                parent[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    // Store parent information even if path not found
    result.parents = parent;
    return result;
}

PathResult AdjacencyList::findPathAStar(NodeID source, NodeID destination, FeaturesStore &features)
{
    PathResult result;
    std::unordered_map<NodeID, NodeID> parent;
    std::unordered_map<NodeID, int> g_score;
    std::unordered_map<NodeID, int> f_score;
    std::unordered_set<NodeID> closed_set;
    std::unordered_set<NodeID> open_set;

    // Get the total number of features the destination has as our reference
    double max_features = features.getFeatures(destination).size();

    // Priority queue with custom comparator
    auto cmp = [&f_score](NodeID a, NodeID b)
    { return f_score[a] > f_score[b]; };
    std::priority_queue<NodeID, std::vector<NodeID>, decltype(cmp)> open_queue(cmp);

    // Initialize scores
    g_score[source] = 0;
    double shared = features.sharedFeatures(source, destination).size();
    double heuristic = max_features - shared;
    f_score[source] = g_score[source] + heuristic;
    result.heuristic_scores[source] = f_score[source];
    parent[source] = -1;
    open_set.insert(source);
    open_queue.push(source);
    result.exploredPath.push_back(source); // Add source to explored path

    while (!open_queue.empty())
    {
        NodeID current = open_queue.top();
        open_queue.pop();

        if (current == destination)
        {
            // Reconstruct path
            NodeID node = destination;
            while (node != -1)
            {
                result.finalPath.push_back(node);
                node = parent[node];
            }
            std::reverse(result.finalPath.begin(), result.finalPath.end());
            result.parents = parent;
            return result;
        }

        open_set.erase(current);
        closed_set.insert(current);

        for (NodeID neighbor : map[current])
        {
            if (closed_set.find(neighbor) != closed_set.end())
            {
                continue;
            }

            int tentative_g_score = g_score[current] + 1;

            if (open_set.find(neighbor) == open_set.end())
            {
                open_set.insert(neighbor);
                result.exploredPath.push_back(neighbor); // Add newly discovered nodes to explored path
            }
            else if (tentative_g_score >= g_score[neighbor])
            {
                continue;
            }

            parent[neighbor] = current;
            g_score[neighbor] = tentative_g_score;
            shared = features.sharedFeatures(neighbor, destination).size();
            heuristic = max_features - shared;
            f_score[neighbor] = g_score[neighbor] + heuristic;
            result.heuristic_scores[neighbor] = f_score[neighbor];
            open_queue.push(neighbor);
        }
    }

    // If we get here, no path was found
    result.parents = parent;
    return result;
}

AdjacencyList AdjacencyList::loadEdgesFromDirectory(std::string path)
{
    AdjacencyList al;
    std::filesystem::path folder = path;
    for (auto &file : std::filesystem::directory_iterator(folder))
    { // iterates through folder
        if (file.is_regular_file() && file.path().extension() == ".edges")
        { // only read .edges files since that is the only one needed
            std::ifstream inFile(file.path());
            std::string line;
            while (std::getline(inFile, line))
            {
                std::istringstream iss(line);
                int u, v; // inthere are two numbers on each line, representing two IDs that are friends
                iss >> u >> v;
                al.addEdge(u, v);
                al.addEdge(v, u);
            }
            inFile.close();
        }
    }
    return al;
}

FeaturesStore FeaturesStore::loadFeaturesFromDirectory(std::string path)
{
    FeaturesStore fs;
    std::filesystem::path folder = path;
    for (auto &file : std::filesystem::directory_iterator(folder))
    { // iterates through folder
        if (file.is_regular_file() && file.path().extension() == ".featnames")
        {
            std::vector<std::string> features;

            // extract feature names
            std::ifstream namesFile(file.path());
            std::string line;
            while (std::getline(namesFile, line))
            {
                std::istringstream iss(line);
                // line looks like "46 #foo"
                int index;
                std::string feature;
                iss >> index >> feature;
                features.push_back(feature);
            }
            namesFile.close();

            // add features
            auto newPath = file.path();
            newPath.replace_extension(".feat");
            std::ifstream featFile(newPath);
            while (std::getline(featFile, line))
            {
                std::istringstream iss(line);
                // line looks like "1186 0 0 0 1 0 0 0 0 0"
                int nodeId;
                iss >> nodeId;
                for (int i = 0; i < features.size(); i++)
                {
                    int hasFeature;
                    iss >> hasFeature;
                    if (hasFeature == 1)
                    {
                        fs.addFeature(nodeId, features[i]);
                    }
                }
            }
            featFile.close();
        }
    }
    return fs;
}

void FeaturesStore::addFeature(NodeID node, std::string feature)
{
    features[node].insert(feature);
}

std::unordered_set<std::string> FeaturesStore::getFeatures(NodeID node)
{
    return features[node];
}

std::unordered_set<std::string> FeaturesStore::sharedFeatures(NodeID node1, NodeID node2)
{
    std::set<std::string> set1(features[node1].begin(), features[node1].end());
    std::set<std::string> set2(features[node2].begin(), features[node2].end());
    std::unordered_set<std::string> shared;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                          std::inserter(shared, shared.begin()));
    return shared;
}

std::unordered_map<NodeID, std::unordered_set<NodeID>> AdjacencyList::getMap()
{
    return map;
}

void FeaturesStore::saveToCache(std::string path)
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        throw std::runtime_error("Failed to open cache file for writing");
    }

    // Write number of nodes
    size_t num_nodes = features.size();
    out.write(reinterpret_cast<const char *>(&num_nodes), sizeof(num_nodes));

    for (const auto &[node, node_features] : features)
    {
        // Write node ID
        out.write(reinterpret_cast<const char *>(&node), sizeof(node));

        // Write number of features for this node
        size_t num_features = node_features.size();
        out.write(reinterpret_cast<const char *>(&num_features), sizeof(num_features));

        // Write each feature
        for (const auto &feature : node_features)
        {
            size_t feature_size = feature.size();
            out.write(reinterpret_cast<const char *>(&feature_size), sizeof(feature_size));
            out.write(feature.c_str(), feature_size);
        }
    }
}

FeaturesStore FeaturesStore::loadFromCache(std::string path)
{
    FeaturesStore fs;
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        throw std::runtime_error("Failed to open cache file for reading");
    }

    // Read number of nodes
    size_t num_nodes;
    in.read(reinterpret_cast<char *>(&num_nodes), sizeof(num_nodes));

    for (size_t i = 0; i < num_nodes; ++i)
    {
        // Read node ID
        NodeID node;
        in.read(reinterpret_cast<char *>(&node), sizeof(node));

        // Read number of features
        size_t num_features;
        in.read(reinterpret_cast<char *>(&num_features), sizeof(num_features));

        // Read each feature
        for (size_t j = 0; j < num_features; ++j)
        {
            size_t feature_size;
            in.read(reinterpret_cast<char *>(&feature_size), sizeof(feature_size));

            std::string feature(feature_size, '\0');
            in.read(&feature[0], feature_size);

            fs.addFeature(node, feature);
        }
    }

    return fs;
}
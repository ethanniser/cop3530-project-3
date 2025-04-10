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

    // If we get here, no path was found
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

    // Priority queue with custom comparator
    auto cmp = [&f_score](NodeID a, NodeID b)
    { return f_score[a] > f_score[b]; };
    std::priority_queue<NodeID, std::vector<NodeID>, decltype(cmp)> open_queue(cmp);

    // Initialize scores
    g_score[source] = 0;
    f_score[source] = features.sharedFeatures(source, destination).size();
    parent[source] = -1;
    open_set.insert(source);
    open_queue.push(source);

    while (!open_queue.empty())
    {
        NodeID current = open_queue.top();
        open_queue.pop();
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
            }
            else if (tentative_g_score >= g_score[neighbor])
            {
                continue;
            }

            parent[neighbor] = current;
            g_score[neighbor] = tentative_g_score;
            f_score[neighbor] = g_score[neighbor] + features.sharedFeatures(neighbor, destination).size();
            open_queue.push(neighbor);
        }
    }

    // If we get here, no path was found
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
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
    throw std::runtime_error("Not implemented");
}

PathResult findPathAStar(NodeID source, NodeID destination, FeaturesStore &features)
{
    throw std::runtime_error("Not implemented");
}

AdjacencyList AdjacencyList::loadEdgesFromDirectory(std::string path)
{
    AdjacencyList al;
    namespace fs = std::filesystem;
    fs::path folder = path;
    for (auto &file : fs::directory_iterator(folder))
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
    namespace fs = std::filesystem;
    fs::path folder = path;
    for (auto &file : fs::directory_iterator(folder))
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
                    std::string feature = features[i];
                    if (iss.peek() == EOF)
                    {
                        break;
                    }
                    iss >> feature;
                    fs.addFeature(nodeId, feature);
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
    std::unordered_set<std::string> shared;
    std::set_union(features[node1].begin(), features[node1].end(), features[node2].begin(), features[node2].end(),
                   std::inserter(shared, shared.begin()));
    return shared;
}
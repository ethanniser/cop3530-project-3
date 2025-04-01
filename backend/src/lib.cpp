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

#include "lib.h"

void AdjacencyList::addEdge(int from, int to)
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

std::vector<int> AdjacencyList::findPathBFS(int source, int destination)
{
    std::queue<int> q;
    std::unordered_set<int> visited;   // intracks visited nodes
    std::unordered_map<int, int> pred; // Map to record predecessor of each node

    q.push(source);
    visited.insert(source);

    bool found = false; // flag to indicate if destination is found

    while (!q.empty() && !found)
    {
        int current = q.front();
        q.pop();

        // iterate over all neighbors of current node
        auto it = map.find(current);
        if (it != map.end())
        {
            for (int neighbor : it->second)
            {
                if (visited.find(neighbor) == visited.end())
                {
                    visited.insert(neighbor);
                    pred[neighbor] = current; // Records how we reached neighbor
                    q.push(neighbor);

                    // Once destination is found, exit loop
                    if (neighbor == destination)
                    {
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    // Reconstruct path
    std::vector<int> path;

    if (found)
    {
        // Start from the destination and move backwards using the parent map.
        int current = destination;
        while (current != source)
        {
            path.push_back(current);
            current = pred[current];
        }
        path.push_back(source);
        // Reverse since we worked up backwards
        std::reverse(path.begin(), path.end());
    }
    return path;
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
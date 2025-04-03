#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class AdjacencyList
{
private:
    std::unordered_map<int, std::unordered_set<int>> map;

public:
    void addEdge(int from, int to);
    std::vector<int> reconstructPath(int source, int destination, std::unordered_map<int, int> pred);
    std::vector<int> findPathBFS(int source, int destination);
    std::vector<int> findPathDFS(int source, int destination);
    static AdjacencyList loadEdgesFromDirectory(std::string path);
};

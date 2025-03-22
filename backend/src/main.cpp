#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <queue>
#include<unordered_set>
#include<algorithm>

#include "crow.h"
#include "lib.h"

namespace fs = std::filesystem;

//Creates the adjacency list that will be used
std::unordered_map<int, std::vector<int>> create_list() {
    std::unordered_map<int, std::vector<int>> adjacencyList;
    fs::path folder = "../data/twitter";
    for(auto& file : fs::directory_iterator(folder)) { //iterates through folder
        if(file.is_regular_file() && file.path().extension() == ".edges") { //only read .edges files since that is the only one needed
            std::ifstream inFile(file.path());
            std::string line;
            while(std::getline(inFile, line)) {
                std::istringstream iss(line);
                int u, v; //There are two numbers on each line, representing two IDs that are friends
                iss >> u >> v;
                adjacencyList[u].push_back(v);
                adjacencyList[v].push_back(u);
            }
            inFile.close();
        }
    }
    return adjacencyList;
}

// This function performs BFS to find the shortest path from 'source' to 'destination'
// in the given graph (adjacency list). It returns a vector containing the path.
// If no path is found, the returned vector will be empty.

std::vector<int> bfs_find_path(std::unordered_map<int, std::vector<int>> &adjacencyList, int source, int destination) {
    std::queue<int> q;
    std::unordered_set<int> visited; //Tracks visited nodes
    std::unordered_map<int, int> pred; //Map to record predecessor of each node

    q.push(source);
    visited.insert(source);

    bool found = false; //flag to indicate if destination is found

    while(!q.empty() && !found) {
        int current = q.front();
        q.pop();

        //iterate over all neighbors of current node
        auto it = adjacencyList.find(current);
        if(it != adjacencyList.end()) {
            for(int neighbor : it->second) {
                if(visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    pred[neighbor] = current; //Records how we reached neighbor
                    q.push(neighbor);

                    //Once destination is found, exit loop
                    if(neighbor == destination) {
                        found = true;
                        break;
                    }
                }
            }
        }
    }

    //Reconstruct path
    std::vector<int> path;

    if(found) {
        // Start from the destination and move backwards using the parent map.
        int current = destination;
        while (current != source) {
            path.push_back(current);
            current = pred[current];
        }
        path.push_back(source);
        //Reverse since we worked up backwards
        std::reverse(path.begin(), path.end());
    }
    return path;
}

int main()
{
    crow::SimpleApp app; // define your crow application

    // define your endpoint at the root directory
    CROW_ROUTE(app, "/")([]()
                         { 
                            std::ostringstream oss;
    oss << "Hello world, foo is " << foo() << "!";
    return oss.str(); });

    // set the port, set the app to run on multiple threads, and run the app
    app.port(8080).concurrency(4).run();

    auto adjacencyList = create_list();


    //Takes two inputs (source and destination) and will find path between them.
    int source, dest;

    std::cout << "Input source node ID:";
    std::cin >> source;

    std::cout << "Input destination ID:";
    std::cin >> dest;

    while(source != -1) {
        std::vector<int> path;
        path = bfs_find_path(adjacencyList, source, dest);

        if(path.empty()) {
            std::cout << "Path not found\n";
        }else {
            std::cout << "Path from source to destination:\n";
            for (size_t i = 0; i < path.size(); ++i) {
                std::cout << path[i];
                if (i < path.size() - 1) {
                    std::cout << " is friends with ";
                }
            }
            std::cout << "\n";
        }

        std::cout << "Input source node ID:";
        std::cin >> source;

        if(source == -1) break;

        std::cout << "Input destination ID:";
        std::cin >> dest;
    }


}
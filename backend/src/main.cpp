#include "crow.h"
#include "lib.h"
#include <sstream>
#include <vector>

int main()
{
    auto adjacencyList = AdjacencyList::loadEdgesFromDirectory("../data/twitter");
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([]()
     { return "Hello world!"; });

    CROW_ROUTE(app, "/path")
    ([&adjacencyList](const crow::request &req)
     {
    auto source_str = req.url_params.get("source");
    auto destination_str = req.url_params.get("destination");

    if (!source_str || !destination_str) {
      return crow::response(400, "Source and destination are required.");
    }

    int source, destination;
    try {
      source = std::stoi(source_str);
      destination = std::stoi(destination_str);
    } catch (const std::exception &e) {
      return crow::response(400, "Invalid source or destination.");
    }

    std::vector<int> path =
        adjacencyList.findPathBFS(source, destination);

    crow::json::wvalue::list json_list;
    for (int node : path) {
        json_list.push_back(node);
    }

    return crow::response{crow::json::wvalue(json_list)}; });

    app.port(8080).concurrency(4).run();
}

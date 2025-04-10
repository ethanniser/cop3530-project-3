#include "crow.h"
#include "lib.h"
#include <sstream>
#include <vector>

/**
 * API:
 *
 * /path
 * Query Params:
 *   source: int (required) - The id of the source node.
 *   destination: int (required) - The id of the destination node.
 *   method: "bfs" or "astar" - The method to use for finding the path.
 *
 * Response:
 *   200 OK
 *   Content-Type: application/json
 *   {
 *     "finalPath": [int], - The path from the source to the destination node, in order
 *     "exploredPath": [int], - The nodes that were explored during the search, in order
 *     "sharedFeatures": [string], - The features that are shared between the source and destination nodes
 *   }
 *
 * /graph
 * Query Params: None
 *
 * Response:
 *   200 OK
 *   Content-Type: application/json
 *   {
 *     "nodes": [
 *       {
 *         "id": int, - The id of the node
 *         "features": [string] - The features associated with the node
 *       },
 *       ...
 *     ],
 *     "edges": [
 *       {
 *         "source": int, - The id of the source node
 *         "target": int - The id of the target node
 *       },
 *       ...
 *     ]
 *   }
 *
 */

int main()
{
  std::cout << "Loading graph..." << std::endl;
  auto adjacencyList = AdjacencyList::loadEdgesFromDirectory("../data/twitter");
  std::cout << "Loading features..." << std::endl;
  auto featuresStore = FeaturesStore::loadFeaturesFromDirectory("../data/twitter");
  std::cout << "Graph loaded successfully." << std::endl;
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")
  ([]()
   { return "Hello world!"; });

  CROW_ROUTE(app, "/graph")
  ([&adjacencyList, &featuresStore]()
   {
    crow::json::wvalue response;
    
    // Convert adjacency list to JSON
    crow::json::wvalue::list nodes_list;
    crow::json::wvalue::list edges_list;
    
    for (const auto& [node, neighbors] : adjacencyList.getMap()) {
      // Add node with its features
      crow::json::wvalue node_obj;
      node_obj["id"] = node;
      auto features = std::vector<std::string>(featuresStore.getFeatures(node).begin(), featuresStore.getFeatures(node).end());
      crow::json::wvalue::list features_list;
      for (const auto& feature : features) {
        features_list.push_back(feature);
      }
      node_obj["features"] = std::move(features_list);
      nodes_list.push_back(node_obj);
      
      // Add edges
      for (const auto& neighbor : neighbors) {
        crow::json::wvalue edge_obj;
        edge_obj["source"] = node;
        edge_obj["target"] = neighbor;
        edges_list.push_back(edge_obj);
      }
    }
    
    response["nodes"] = std::move(nodes_list);
    response["edges"] = std::move(edges_list);
    
    return crow::response{response}; });

  CROW_ROUTE(app, "/path")
  ([&adjacencyList, &featuresStore](const crow::request &req)
   {
    auto source_str = req.url_params.get("source");
    auto destination_str = req.url_params.get("destination");
    auto method_str = req.url_params.get("method");

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

    PathResult result;
    if (!method_str || std::string(method_str) == "bfs") {
      result = adjacencyList.findPathBFS(source, destination);
    } else if (std::string(method_str) == "astar") {
      result = adjacencyList.findPathAStar(source, destination, featuresStore);
    } else {
      return crow::response(400, "Invalid method. Use 'bfs' or 'astar'.");
    }

    // Get shared features
    auto shared_features = featuresStore.sharedFeatures(source, destination);
    
    // Create response
    crow::json::wvalue response;
    response["finalPath"] = result.finalPath;
    response["exploredPath"] = result.exploredPath;
    
    // Convert shared features to JSON array
    crow::json::wvalue::list shared_features_list;
    for (const auto& feature : shared_features) {
      shared_features_list.push_back(crow::json::wvalue(feature));
    }
    response["sharedFeatures"] = std::move(shared_features_list);

    return crow::response{response}; });

  app.port(8080).concurrency(4).run();
}

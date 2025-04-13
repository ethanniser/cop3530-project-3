#include "crow.h"
#include "lib.h"
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>

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
 *     "parents": {string: int}, - Maps each node ID to its parent node ID in the search tree (-1 for source)
 *     "sharedFeatures": [string], - The features that are shared between the source and destination nodes
 *   }
 */

crow::response addCorsHeaders(crow::response &&response)
{
  response.add_header("Access-Control-Allow-Origin", "*");
  response.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response.add_header("Access-Control-Allow-Headers", "Content-Type");
  return std::move(response);
}

int main()
{
  std::cout << "Loading graph..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  auto adjacencyList = AdjacencyList::loadEdgesFromDirectory("../data/twitter");
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "Graph loaded in " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds" << std::endl;

  std::cout << "Loading features..." << std::endl;
  start = std::chrono::high_resolution_clock::now();

  FeaturesStore featuresStore;
  std::string cache_path = "./.cache/features";
  try
  {
    featuresStore = FeaturesStore::loadFromCache(cache_path);
    std::cout << "Loaded features from cache" << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cout << "Cache not found or invalid, loading from source files..." << std::endl;
    featuresStore = FeaturesStore::loadFeaturesFromDirectory("../data/twitter");
    std::cout << "Saving features to cache..." << std::endl;
    featuresStore.saveToCache(cache_path);
  }

  end = std::chrono::high_resolution_clock::now();
  elapsed = end - start;
  std::cout << "Features loaded in " << std::fixed << std::setprecision(2) << elapsed.count() << " seconds" << std::endl;

  std::cout << "Graph loaded successfully." << std::endl;
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")
  ([]()
   { return addCorsHeaders(crow::response("Hello world!")); });

  CROW_ROUTE(app, "/path")
  ([&adjacencyList, &featuresStore](const crow::request &req)
   {
    auto source_str = req.url_params.get("source");
    auto destination_str = req.url_params.get("destination");
    auto method_str = req.url_params.get("method");

    if (!source_str || !destination_str) {
      return addCorsHeaders(crow::response(400, "Source and destination are required."));
    }

    int source, destination;
    try {
      source = std::stoi(source_str);
      destination = std::stoi(destination_str);
    } catch (const std::exception &e) {
      return addCorsHeaders(crow::response(400, "Invalid source or destination."));
    }

    PathResult result;
    if (!method_str || std::string(method_str) == "bfs") {
      result = adjacencyList.findPathBFS(source, destination);
    } else if (std::string(method_str) == "astar") {
      result = adjacencyList.findPathAStar(source, destination, featuresStore);
    } else {
      return addCorsHeaders(crow::response(400, "Invalid method. Use 'bfs' or 'astar'."));
    }

    // Get shared features
    auto shared_features = featuresStore.sharedFeatures(source, destination);
    
    // Create response
    crow::json::wvalue response;
    response["finalPath"] = result.finalPath;
    response["exploredPath"] = result.exploredPath;
    
    // Convert parents map to JSON object
    crow::json::wvalue parents_obj;
    for (const auto& [child, parent] : result.parents) {
        parents_obj[std::to_string(child)] = parent;
    }
    response["parents"] = std::move(parents_obj);
    
    // Convert shared features to JSON array
    crow::json::wvalue::list shared_features_list;
    for (const auto& feature : shared_features) {
        shared_features_list.push_back(crow::json::wvalue(feature));
    }
    response["sharedFeatures"] = std::move(shared_features_list);

    return addCorsHeaders(crow::response{response}); });

  // Add OPTIONS handler for CORS preflight requests
  CROW_ROUTE(app, "/path")
      .methods("OPTIONS"_method)([]()
                                 { return addCorsHeaders(crow::response(200)); });

  app.port(8080).concurrency(4).run();
}

#include "crow.h"
#include "lib.h"
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

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
 *
 * /stats
 * Description: Returns statistics about the graph, including total number of nodes and edges.
 * Method: GET
 * No query parameters required.
 *
 * Response:
 *   200 OK
 *   Content-Type: application/json
 *   {
 *     "nodes": int, - Total number of nodes in the graph
 *     "edges": int  - Total number of edges in the graph (each bidirectional edge counted once)
 *   }
 */

crow::response addCorsHeaders(crow::response &&response)
{
  response.add_header("Access-Control-Allow-Origin", "*");
  response.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response.add_header("Access-Control-Allow-Headers", "Content-Type");
  return std::move(response);
}

// Convert PathResult to JSON
crow::json::wvalue pathResultToJson(const PathResult &result, FeaturesStore &features, NodeID source, NodeID destination)
{
  crow::json::wvalue::list finalPath;
  for (const auto &node : result.finalPath)
  {
    finalPath.push_back(node);
  }

  crow::json::wvalue::list exploredPath;
  for (const auto &node : result.exploredPath)
  {
    exploredPath.push_back(node);
  }

  crow::json::wvalue::object parents;
  for (const auto &[child, parent] : result.parents)
  {
    parents[std::to_string(child)] = parent;
  }

  crow::json::wvalue::object heuristicScores;
  for (const auto &[node, score] : result.heuristic_scores)
  {
    heuristicScores[std::to_string(node)] = score;
  }

  crow::json::wvalue::object sharedWithSource;
  for (const auto &[node, count] : result.shared_with_source)
  {
    sharedWithSource[std::to_string(node)] = count;
  }

  std::vector<std::string> sharedFeatures;
  if (source != -1 && destination != -1)
  {
    auto features_set = features.sharedFeatures(source, destination);
    sharedFeatures = std::vector<std::string>(features_set.begin(), features_set.end());
  }

  crow::json::wvalue::list sharedFeaturesList;
  for (const auto &feature : sharedFeatures)
  {
    sharedFeaturesList.push_back(feature);
  }

  return crow::json::wvalue{
      {"finalPath", std::move(finalPath)},
      {"exploredPath", std::move(exploredPath)},
      {"parents", std::move(parents)},
      {"heuristic_scores", std::move(heuristicScores)},
      {"shared_with_source", std::move(sharedWithSource)},
      {"sharedFeatures", std::move(sharedFeaturesList)}};
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

  CROW_ROUTE(app, "/stats")
  ([&adjacencyList](const crow::request &)
   {
          crow::json::wvalue response;
          response["nodes"] = adjacencyList.getNodeCount();
          response["edges"] = adjacencyList.getEdgeCount();
          return addCorsHeaders(crow::response(std::move(response))); });

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

    auto json = pathResultToJson(result, featuresStore, source, destination);
    return addCorsHeaders(crow::response{json}); });

  // Add OPTIONS handler for CORS preflight requests
  CROW_ROUTE(app, "/path")
      .methods("OPTIONS"_method)([]()
                                 { return addCorsHeaders(crow::response(200)); });

  CROW_ROUTE(app, "/stats")
      .methods("OPTIONS"_method)([]()
                                 { return addCorsHeaders(crow::response(200)); });

  app.port(8080).run();
}

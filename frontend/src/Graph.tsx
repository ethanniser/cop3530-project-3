import React, { useEffect, useState } from "react";
import ForceGraph2D from "react-force-graph-2d";

interface Node {
  id: number;
  features: string[];
}

interface Edge {
  source: number;
  target: number;
}

interface GraphData {
  nodes: Node[];
  edges: Edge[];
}

interface PathResult {
  finalPath: number[];
  exploredPath: number[];
  sharedFeatures: string[];
}

const Graph: React.FC = () => {
  const [graphData, setGraphData] = useState<GraphData | null>(null);
  const [source, setSource] = useState<number | null>(null);
  const [destination, setDestination] = useState<number | null>(null);
  const [pathResult, setPathResult] = useState<PathResult | null>(null);
  const [method, setMethod] = useState<"bfs" | "astar">("bfs");

  useEffect(() => {
    // Fetch graph data
    fetch("http://localhost:8080/graph")
      .then((res) => res.json())
      .then((data) => setGraphData(data))
      .catch((err) => console.error("Error fetching graph:", err));
  }, []);

  const findPath = () => {
    if (!source || !destination) return;

    fetch(
      `http://localhost:8080/path?source=${source}&destination=${destination}&method=${method}`
    )
      .then((res) => res.json())
      .then((data) => setPathResult(data))
      .catch((err) => console.error("Error finding path:", err));
  };

  if (!graphData) return <div>Loading graph...</div>;

  return (
    <div className="flex flex-col items-center p-4">
      <div className="mb-4 flex gap-4">
        <input
          type="number"
          placeholder="Source node"
          value={source || ""}
          onChange={(e) => setSource(Number(e.target.value))}
          className="border p-2 rounded"
        />
        <input
          type="number"
          placeholder="Destination node"
          value={destination || ""}
          onChange={(e) => setDestination(Number(e.target.value))}
          className="border p-2 rounded"
        />
        <select
          value={method}
          onChange={(e) => setMethod(e.target.value as "bfs" | "astar")}
          className="border p-2 rounded"
        >
          <option value="bfs">BFS</option>
          <option value="astar">A*</option>
        </select>
        <button
          onClick={findPath}
          className="bg-blue-500 text-white px-4 py-2 rounded hover:bg-blue-600"
        >
          Find Path
        </button>
      </div>

      <div className="w-full h-[600px] border rounded">
        <ForceGraph2D
          graphData={graphData}
          nodeLabel={(node: Node) => `Node ${node.id}`}
          nodeColor={(node: Node) => {
            if (pathResult?.finalPath.includes(node.id)) return "red";
            if (pathResult?.exploredPath.includes(node.id)) return "yellow";
            return "blue";
          }}
          linkColor={() => "gray"}
          nodeRelSize={6}
        />
      </div>

      {pathResult && (
        <div className="mt-4">
          <h3 className="text-lg font-bold">Path Result</h3>
          <p>Final Path: {pathResult.finalPath.join(" -> ")}</p>
          <p>Explored Nodes: {pathResult.exploredPath.length}</p>
          <p>Shared Features: {pathResult.sharedFeatures.join(", ")}</p>
        </div>
      )}
    </div>
  );
};

export default Graph;

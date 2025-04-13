import React from "react";
import { useQuery } from "@tanstack/react-query";
import ForceGraph2D from "react-force-graph-2d";

interface PathResult {
  finalPath: number[];
  exploredPath: number[];
  parents: { [key: number]: number };
  sharedFeatures: string[];
  heuristic_scores: { [key: number]: number };
  shared_with_source: { [key: number]: number };
}

interface GraphNode {
  id: number;
  isFinalPath: boolean;
  heuristicScore?: number;
  sharedWithSource?: number;
  level?: number;
  x?: number;
}

interface GraphLink {
  source: number;
  target: number;
  isFinalPath: boolean;
  heuristicScore?: number;
}

interface GraphData {
  nodes: GraphNode[];
  links: GraphLink[];
}

interface GraphProps {
  source: number | null;
  destination: number | null;
  method: "bfs" | "astar";
}

const fetchPath = async (
  source: number,
  destination: number,
  method: string
): Promise<PathResult> => {
  const response = await fetch(
    `http://localhost:8080/path?source=${source}&destination=${destination}&method=${method}`
  );
  if (!response.ok) {
    throw new Error("Network response was not ok");
  }
  return response.json();
};

const getHeuristicColor = (score: number, maxScore: number) => {
  // Convert score to a color from blue (low) to red (high)
  const normalizedScore = Math.max(0, Math.min(1, score / maxScore));
  const r = Math.round(255 * normalizedScore);
  const b = Math.round(255 * (1 - normalizedScore));
  return `rgb(${r}, 0, ${b})`;
};

const Graph: React.FC<GraphProps> = ({ source, destination, method }) => {
  const { data: bfsResult } = useQuery({
    queryKey: ["path", source, destination, "bfs"],
    queryFn: () => fetchPath(source!, destination!, "bfs"),
    enabled: !!source && !!destination,
  });

  const { data: astarResult } = useQuery({
    queryKey: ["path", source, destination, "astar"],
    queryFn: () => fetchPath(source!, destination!, "astar"),
    enabled: !!source && !!destination,
  });

  const pathResult = method === "bfs" ? bfsResult : astarResult;

  const getComparisonText = (currentCount: number, otherCount: number) => {
    const diff = ((currentCount - otherCount) / otherCount) * 100;
    if (diff > 0) {
      return `(${diff.toFixed(0)}% increase compared to ${
        method === "bfs" ? "A*" : "BFS"
      })`;
    } else if (diff < 0) {
      return `(${Math.abs(diff).toFixed(0)}% decrease compared to ${
        method === "bfs" ? "A*" : "BFS"
      })`;
    }
    return `(same as ${method === "bfs" ? "A*" : "BFS"})`;
  };

  const graphRef = React.useRef<HTMLDivElement>(null);
  const [dimensions, setDimensions] = React.useState({ width: 0, height: 0 });

  React.useEffect(() => {
    const updateDimensions = () => {
      if (graphRef.current) {
        setDimensions({
          width: graphRef.current.offsetWidth,
          height: graphRef.current.offsetHeight,
        });
      }
    };

    updateDimensions();
    window.addEventListener("resize", updateDimensions);
    return () => window.removeEventListener("resize", updateDimensions);
  }, []);

  // Create graph data from the path results
  const graphData = React.useMemo<GraphData>(() => {
    if (!pathResult) return { nodes: [], links: [] };

    // Create a set of nodes in the final path for quick lookup
    const finalPathNodes = new Set(pathResult.finalPath);

    // Create nodes and calculate their levels
    const nodes: GraphNode[] = [];
    const nodeSet = new Set<number>();

    // First, ensure we have all nodes that appear in the parent relationships
    for (const [childStr, parent] of Object.entries(pathResult.parents)) {
      const child = parseInt(childStr);

      // Add child node if not already added
      if (!nodeSet.has(child)) {
        nodes.push({
          id: child,
          isFinalPath: finalPathNodes.has(child),
          heuristicScore: pathResult.heuristic_scores?.[child],
          sharedWithSource: pathResult.shared_with_source?.[child],
        });
        nodeSet.add(child);
      }

      // Add parent node if not already added and it's not -1 (source's parent)
      if (parent !== -1 && !nodeSet.has(parent)) {
        nodes.push({
          id: parent,
          isFinalPath: finalPathNodes.has(parent),
          heuristicScore: pathResult.heuristic_scores?.[parent],
          sharedWithSource: pathResult.shared_with_source?.[parent],
        });
        nodeSet.add(parent);
      }
    }

    // Create links based on parent relationships
    const links: GraphLink[] = [];
    for (const [childStr, parent] of Object.entries(pathResult.parents)) {
      if (parent !== -1) {
        // Skip source node which has no parent
        const child = parseInt(childStr);
        const isInFinalPath =
          finalPathNodes.has(child) && finalPathNodes.has(parent);
        links.push({
          source: parent,
          target: child,
          isFinalPath: isInFinalPath,
          heuristicScore: pathResult.heuristic_scores?.[child],
        });
      }
    }

    return { nodes, links };
  }, [pathResult]);

  return (
    <div className="flex flex-col h-[calc(100vh-200px)]">
      {!source || !destination ? (
        <div className="flex flex-col items-center p-4">
          <p className="text-gray-500">
            Enter source and destination IDs to find a path
          </p>
        </div>
      ) : pathResult && bfsResult && astarResult ? (
        <div className="p-4 bg-white rounded-lg shadow mb-4">
          <h3 className="text-lg font-bold mb-2">Path Result</h3>
          <div className="space-y-2">
            <p>
              <span className="font-semibold">Final Path:</span>{" "}
              {pathResult.finalPath.join(" → ")}
            </p>
            <p>
              <span className="font-semibold">Explored Nodes:</span>{" "}
              {pathResult.exploredPath.length}{" "}
              <span className="text-gray-500">
                {getComparisonText(
                  pathResult.exploredPath.length,
                  method === "bfs"
                    ? astarResult.exploredPath.length
                    : bfsResult.exploredPath.length
                )}
              </span>
            </p>
            <p>
              <span className="font-semibold">Shared Features:</span>{" "}
              {pathResult.sharedFeatures?.join(", ")}
            </p>
            {method === "astar" && (
              <div className="flex items-center gap-2 mt-2">
                <span className="font-semibold">Heuristic Scale:</span>
                <div className="w-32 h-4 bg-gradient-to-r from-blue-500 to-red-500 rounded" />
                <span className="text-sm">Low → High Cost</span>
              </div>
            )}
          </div>
        </div>
      ) : (
        <div className="flex flex-col items-center p-4">
          <p className="text-gray-500">Searching for path...</p>
        </div>
      )}

      <div ref={graphRef} className="flex-grow border rounded overflow-hidden">
        <ForceGraph2D
          graphData={graphData}
          width={dimensions.width}
          height={dimensions.height}
          nodeLabel={(node: GraphNode) =>
            `Node ${node.id}${
              node.heuristicScore !== undefined
                ? ` (cost=${node.heuristicScore.toFixed(2)})`
                : ""
            }${
              node.sharedWithSource !== undefined
                ? `\nShared features with source: ${node.sharedWithSource}`
                : ""
            }`
          }
          nodeColor={(node: GraphNode) => {
            if (node.isFinalPath) {
              return "#22c55e"; // Green color for final path
            }
            if (method === "astar" && node.heuristicScore !== undefined) {
              const scores = Object.values(pathResult?.heuristic_scores || {});
              const maxScore = Math.max(...scores);
              const minScore = Math.min(...scores);
              return getHeuristicColor(
                node.heuristicScore - minScore,
                maxScore - minScore
              );
            }
            return "blue";
          }}
          linkColor={(link: GraphLink) => {
            if (link.isFinalPath) {
              return "#22c55e"; // Green color for final path
            }
            if (method === "astar" && link.heuristicScore !== undefined) {
              const scores = Object.values(pathResult?.heuristic_scores || {});
              const maxScore = Math.max(...scores);
              const minScore = Math.min(...scores);
              return getHeuristicColor(
                link.heuristicScore - minScore,
                maxScore - minScore
              );
            }
            return "#cccccc";
          }}
          nodeRelSize={8}
          linkWidth={(link) => (link.isFinalPath ? 3 : 1)}
          linkDirectionalParticles={(link) => (link.isFinalPath ? 4 : 0)}
          linkDirectionalParticleWidth={2}
          dagMode="td"
          dagLevelDistance={200}
          d3VelocityDecay={0.3}
          cooldownTime={3000}
          d3AlphaMin={0.1}
        />
      </div>
    </div>
  );
};

export default Graph;

import React from "react";
import { useQuery } from "@tanstack/react-query";
import ForceGraph2D from "react-force-graph-2d";

interface PathResult {
  finalPath: number[];
  exploredPath: number[];
  parents: { [key: number]: number };
  sharedFeatures: string[];
}

interface GraphNode {
  id: number;
  isFinalPath: boolean;
  level?: number;
  x?: number;
}

interface GraphLink {
  source: number;
  target: number;
  isFinalPath: boolean;
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

const Graph: React.FC<GraphProps> = ({ source, destination, method }) => {
  const { data: pathResult } = useQuery({
    queryKey: ["path", source, destination, method],
    queryFn: () => fetchPath(source!, destination!, method),
    enabled: !!source && !!destination,
  });

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
    const nodeLevels = new Map<number, number>();

    // Calculate levels for each node based on BFS parent relationships
    const calculateLevels = (nodeId: number, level: number) => {
      nodeLevels.set(nodeId, level);
      for (const [child, parent] of Object.entries(pathResult.parents)) {
        if (parent === nodeId) {
          calculateLevels(Number(child), level + 1);
        }
      }
    };

    // Start from source node (level 0)
    calculateLevels(source!, 0);

    // Create nodes with level information
    for (const id of pathResult.exploredPath) {
      if (!nodeSet.has(id)) {
        nodes.push({
          id,
          isFinalPath: finalPathNodes.has(id),
          level: nodeLevels.get(id) || 0,
        });
        nodeSet.add(id);
      }
    }

    // Create links based on parent relationships
    const links: GraphLink[] = [];
    for (const [child, parent] of Object.entries(pathResult.parents)) {
      if (parent !== -1) {
        // Skip source node which has no parent
        const isInFinalPath =
          finalPathNodes.has(Number(child)) && finalPathNodes.has(parent);
        links.push({
          source: parent,
          target: Number(child),
          isFinalPath: isInFinalPath,
        });
      }
    }

    return { nodes, links };
  }, [pathResult, source]);

  if (!source || !destination) {
    return (
      <div className="flex flex-col items-center p-4">
        <p className="text-gray-500">
          Enter source and destination IDs to find a path
        </p>
      </div>
    );
  }

  return (
    <div className="flex flex-col h-[calc(100vh-200px)]">
      {pathResult ? (
        <>
          <div className="p-4 bg-white rounded-lg shadow mb-4">
            <h3 className="text-lg font-bold mb-2">Path Result</h3>
            <div className="space-y-2">
              <p>
                <span className="font-semibold">Final Path:</span>{" "}
                {pathResult.finalPath.join(" → ")}
              </p>
              <p>
                <span className="font-semibold">Explored Nodes:</span>{" "}
                {pathResult.exploredPath.length}
              </p>
              <p>
                <span className="font-semibold">Shared Features:</span>{" "}
                {pathResult.sharedFeatures.join(", ")}
              </p>
            </div>
          </div>
          <div
            ref={graphRef}
            className="flex-grow border rounded overflow-hidden"
          >
            <ForceGraph2D
              graphData={graphData}
              width={dimensions.width}
              height={dimensions.height}
              nodeLabel={(node: GraphNode) => `Node ${node.id}`}
              nodeColor={(node: GraphNode) =>
                node.isFinalPath ? "red" : "blue"
              }
              linkColor={(link: GraphLink) =>
                link.isFinalPath ? "red" : "#cccccc"
              }
              nodeRelSize={8}
              linkWidth={(link) => (link.isFinalPath ? 3 : 1)}
              linkDirectionalParticles={(link) => (link.isFinalPath ? 4 : 0)}
              linkDirectionalParticleWidth={2}
              dagMode="td"
              dagLevelDistance={100}
              d3VelocityDecay={0.3}
            />
          </div>
        </>
      ) : (
        <p className="text-gray-500">Searching for path...</p>
      )}
    </div>
  );
};

export default Graph;

import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { Suspense, useState } from "react";
import Graph from "./Graph";
import { useQuery } from "@tanstack/react-query";

const queryClient = new QueryClient();

// Example node IDs
const exampleNodes = [
  59588845, 69592091, 250178329, 260769396, 276577539, 279787626, 314316607,
  447688115, 461410856, 529007327,
];

// Example pairs for quick loading
const examples = [
  {
    source: 314316607,
    destination: 250178329,
    note: "A* can be much faster than BFS by prioritizing paths with shared interests",
  },
  {
    source: 250178329,
    destination: 529007327,
    note: "A* isn't always faster",
  },
  {
    source: 69592091,
    destination: 59588845,
    note: 'With nothing in common, BFS may be faster as it explores more "diverse" paths',
  },
];

interface GraphStats {
  nodes: number;
  edges: number;
}

function AppMain() {
  const [source, setSource] = useState<number | null>(null);
  const [destination, setDestination] = useState<number | null>(null);
  const [method, setMethod] = useState<"bfs" | "astar">("bfs");
  const [activeExample, setActiveExample] = useState<number | null>(null);

  const { data: stats } = useQuery<GraphStats>({
    queryKey: ["stats"],
    queryFn: async () => {
      const response = await fetch("http://localhost:8080/stats");
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      return response.json();
    },
  });

  const loadExample = (index: number) => {
    setSource(examples[index].source);
    setDestination(examples[index].destination);
    setActiveExample(index);
  };

  return (
    <div className="min-h-screen bg-gray-100">
      <header className="bg-white shadow">
        <div className="max-w-7xl mx-auto py-6 px-4 sm:px-6 lg:px-8">
          <h1 className="text-3xl font-bold text-gray-900">
            DSA Project 3 - Pathfinding Connections Between Twitter Users (BFS
            vs A*)
          </h1>
          {stats && (
            <p className="mt-2 text-sm text-gray-600">
              Graph contains {stats.nodes.toLocaleString()} nodes and{" "}
              {stats.edges.toLocaleString()} edges
            </p>
          )}
        </div>
      </header>
      <main className="max-w-7xl mx-auto py-6 sm:px-6 lg:px-8">
        <div className="mb-4 flex gap-4 items-center flex-wrap">
          <div className="relative">
            <input
              type="number"
              list="sourceNodes"
              placeholder="Source node"
              value={source || ""}
              onChange={(e) => {
                setSource(e.target.value ? Number(e.target.value) : null);
                setActiveExample(null);
              }}
              className="border p-2 rounded pr-8"
            />
            {source !== null && (
              <button
                onClick={() => {
                  setSource(null);
                  setActiveExample(null);
                }}
                className="absolute right-2 top-1/2 -translate-y-1/2 text-gray-400 hover:text-gray-600"
              >
                ×
              </button>
            )}
            <datalist id="sourceNodes">
              {exampleNodes.map((nodeId) => (
                <option key={nodeId} value={nodeId} />
              ))}
            </datalist>
          </div>
          <div className="relative">
            <input
              type="number"
              list="destinationNodes"
              placeholder="Destination node"
              value={destination || ""}
              onChange={(e) => {
                setDestination(e.target.value ? Number(e.target.value) : null);
                setActiveExample(null);
              }}
              className="border p-2 rounded pr-8"
            />
            {destination !== null && (
              <button
                onClick={() => {
                  setDestination(null);
                  setActiveExample(null);
                }}
                className="absolute right-2 top-1/2 -translate-y-1/2 text-gray-400 hover:text-gray-600"
              >
                ×
              </button>
            )}
            <datalist id="destinationNodes">
              {exampleNodes.map((nodeId) => (
                <option key={nodeId} value={nodeId} />
              ))}
            </datalist>
          </div>
          <select
            value={method}
            onChange={(e) => setMethod(e.target.value as "bfs" | "astar")}
            className="border p-2 rounded"
          >
            <option value="bfs">BFS</option>
            <option value="astar">A*</option>
          </select>
          <div className="flex gap-2">
            {examples.map((_, i) => (
              <button
                key={i}
                onClick={() => loadExample(i)}
                className="px-3 py-2 bg-blue-500 text-white rounded hover:bg-blue-600 transition-colors"
              >
                Load Example {i + 1}
              </button>
            ))}
          </div>
        </div>

        <Suspense
          fallback={
            <div className="w-full h-[600px] border rounded flex items-center justify-center">
              <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-blue-500"></div>
            </div>
          }
        >
          {!source || !destination ? (
            <div className="w-full h-[600px] border rounded flex items-center justify-center">
              <p className="text-gray-500">
                Enter source and destination IDs to find a path
              </p>
            </div>
          ) : (
            <div>
              {activeExample !== null && (
                <p className="text-gray-500 mb-4">
                  Example {activeExample + 1}: {examples[activeExample].note}
                </p>
              )}
              <Graph
                source={source}
                destination={destination}
                method={method}
              />
            </div>
          )}
        </Suspense>
      </main>
    </div>
  );
}

export default function App() {
  return (
    <QueryClientProvider client={queryClient}>
      <AppMain />
    </QueryClientProvider>
  );
}

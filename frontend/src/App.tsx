import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { Suspense, useState } from "react";
import Graph from "./Graph";

const queryClient = new QueryClient();

// Example node IDs
const exampleNodes = [
  59588845, 69592091, 250178329, 260769396, 276577539, 279787626, 314316607,
  447688115, 461410856, 529007327,
];

function App() {
  const [source, setSource] = useState<number | null>(null);
  const [destination, setDestination] = useState<number | null>(null);
  const [method, setMethod] = useState<"bfs" | "astar">("bfs");

  return (
    <QueryClientProvider client={queryClient}>
      <div className="min-h-screen bg-gray-100">
        <header className="bg-white shadow">
          <div className="max-w-7xl mx-auto py-6 px-4 sm:px-6 lg:px-8">
            <h1 className="text-3xl font-bold text-gray-900">
              Graph Path Finder
            </h1>
          </div>
        </header>
        <main className="max-w-7xl mx-auto py-6 sm:px-6 lg:px-8">
          <div className="mb-4 flex gap-4">
            <div className="relative">
              <input
                type="number"
                list="sourceNodes"
                placeholder="Source node"
                value={source || ""}
                onChange={(e) =>
                  setSource(e.target.value ? Number(e.target.value) : null)
                }
                className="border p-2 rounded pr-8"
              />
              {source !== null && (
                <button
                  onClick={() => setSource(null)}
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
                onChange={(e) =>
                  setDestination(e.target.value ? Number(e.target.value) : null)
                }
                className="border p-2 rounded pr-8"
              />
              {destination !== null && (
                <button
                  onClick={() => setDestination(null)}
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
          </div>

          <Suspense
            fallback={
              <div className="w-full h-[600px] border rounded flex items-center justify-center">
                <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-blue-500"></div>
              </div>
            }
          >
            <Graph source={source} destination={destination} method={method} />
          </Suspense>
        </main>
      </div>
    </QueryClientProvider>
  );
}

export default App;

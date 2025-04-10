import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { Suspense, useState } from "react";
import Graph from "./Graph";

const queryClient = new QueryClient();

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

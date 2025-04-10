import cytoscape from "cytoscape";
import { useEffect, useRef } from "react";

export function Graph() {
  const cyRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (cyRef.current) {
      const cy = cytoscape({
        container: cyRef.current,
        elements: [
          { data: { id: "a", label: "A" } },
          { data: { id: "b", label: "B" } },
          { data: { id: "c", label: "C" } },
          { data: { id: "d", label: "D" } },
          { data: { id: "e", label: "E" } },
          { data: { id: "f", label: "F" } },
          { data: { id: "g", label: "G" } },
        ],
        layout: {
          name: "grid",
        },
      });

      cy.on("tap", "node", function (evt) {
        console.log(evt.target.data());
      });

      // Function to update Cytoscape's size
      const updateCySize = () => {
        cy.resize();
      };

      // Initial resize
      updateCySize();

      // Resize on window resize
      window.addEventListener("resize", updateCySize);

      // Cleanup listener on unmount
      return () => {
        window.removeEventListener("resize", updateCySize);
      };
    }
  }, []);

  return (
    <div
      ref={cyRef}
      style={{
        height: "calc(100vh - 64px)", // Adjust 64px to account for header
        width: "100%",
      }}
    />
  );
}

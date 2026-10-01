import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import tailwindcss from "@tailwindcss/vite";

export default defineConfig({
  plugins: [react(), tailwindcss()],
  server: {
    port: 5173,
    proxy: { "/api": "http://127.0.0.1:4321" },
  },
  build: {
    // Monaco is big and ships its own workers + ~100 CSS files; both warnings are expected noise.
    chunkSizeWarningLimit: 5000,
    rolldownOptions: { checks: { bundlerTimings: false } },
  },
});

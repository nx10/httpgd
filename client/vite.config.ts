import { defineConfig } from "vite";
import { svelte } from "@sveltejs/vite-plugin-svelte";
import tailwindcss from "@tailwindcss/vite";
import path from "path";

export default defineConfig({
  plugins: [svelte(), tailwindcss()],
  base: "./",
  resolve: {
    alias: {
      $lib: path.resolve("./src/lib"),
    },
  },
  build: {
    outDir: "../inst/www",
    emptyOutDir: true,
    rollupOptions: {
      output: {
        entryFileNames: "bundle.js",
        chunkFileNames: "[name].js",
        assetFileNames: "[name][extname]",
      },
    },
  },
  server: {
    port: 9000,
    proxy: Object.fromEntries(
      [
        "/state",
        "/plots",
        "/plot",
        "/renderers",
        "/info",
        "/remove",
        "/clear",
      ].map((route) => [
        route,
        `http://127.0.0.1:${process.env.HTTPGD_PORT ?? 8080}`,
      ]),
    ),
  },
});

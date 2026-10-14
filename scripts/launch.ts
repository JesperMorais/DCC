import { spawn } from "node:child_process";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { RESTART_CODE } from "../server/update.ts";

/**
 * Runs the server and starts it again when it exits with RESTART_CODE, which it does after
 * updating itself from the Update button. Any other exit ends the launcher with the same code.
 */
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const tsx = path.join(root, "node_modules", ".bin", "tsx");

function start() {
  const child = spawn(tsx, ["server/index.ts"], { cwd: root, stdio: "inherit", env: { ...process.env, DAILY_TS_LAUNCHER: "1" } });
  child.on("exit", (code, signal) => {
    if (code === RESTART_CODE) {
      console.log("\n  daily.ts updated, restarting…");
      start();
    } else process.exit(code ?? (signal ? 1 : 0));
  });
  // Ctrl+C reaches the server directly (same process group); just pass on a plain kill.
  process.removeAllListeners("SIGTERM");
  process.on("SIGTERM", () => child.kill("SIGTERM"));
}

process.on("SIGINT", () => {}); // let the server handle it and exit; we follow
start();

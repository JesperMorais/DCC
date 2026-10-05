// The entry point for `npm start`. Already done: it hands the words you typed
// to `run` (src/cli.ts) and prints what comes back. The list lives in
// todos.json in the folder you run it from.
import { run } from "./cli.ts";

console.log(run(process.argv.slice(2), "todos.json"));

// Entry point: `npm start -- <command> <file.csv> [args] [flags]`
//
// process.argv.slice(2) holds the arguments, e.g. ["report", "bank.csv", "2026-09", "--rules", "rules.txt"].
// Print results with console.log (stdout), warnings and errors with console.error (stderr),
// and set process.exitCode = 1 when something went wrong.
//
// The milestones in the app describe each command's exact output. Split the work
// into modules as you see fit (the README suggests a layout).

// The data types are in ./types.ts.

const args = process.argv.slice(2);

// TODO (milestone 1): replace this with your CLI.
console.error(`not implemented yet: ${args.join(" ")}`);
process.exitCode = 1;

import type { Command, Flags } from "./types.ts";

export const USAGE = "usage: expenses <list|report|top|months> <file.csv> [report: YYYY-MM | top: n] [--rules file] [--month YYYY-MM]";

const COMMANDS: readonly Command[] = ["list", "report", "top", "months"];
/** How many positional arguments each command takes, the CSV file included. */
const ARITY: Record<Command, number> = { list: 1, report: 2, top: 2, months: 1 };
const FLAGS: readonly (keyof Flags)[] = ["rules", "month"];

export interface Invocation {
  command: Command;
  positional: string[];
  flags: Flags;
}

const isCommand = (s: string | undefined): s is Command => COMMANDS.some((c) => c === s);
const isFlag = (s: string): s is keyof Flags => FLAGS.some((f) => f === s);

/** Splits argv into command, positionals and flags, or says what's wrong with it. */
export function parseArgs(argv: readonly string[]): Invocation | { error: string } {
  const [command, ...rest] = argv;
  if (!isCommand(command)) return { error: USAGE };
  const positional: string[] = [];
  const flags: Flags = {};
  for (let i = 0; i < rest.length; i++) {
    const arg = rest[i]!;
    if (!arg.startsWith("--")) {
      positional.push(arg);
      continue;
    }
    const name = arg.slice(2);
    if (!isFlag(name)) return { error: `error: unknown flag ${arg}` };
    const value = rest[i + 1];
    if (value === undefined || value.startsWith("--")) return { error: `error: ${arg} needs a value` };
    flags[name] = value;
    i++;
  }
  if (positional.length !== ARITY[command]) return { error: USAGE };
  return { command, positional, flags };
}

import { readFileSync } from "node:fs";
import { parseArgs } from "./args.ts";
import * as commands from "./commands.ts";
import { parseCsv } from "./csv.ts";
import { parseRules } from "./rules.ts";
import type { Rule } from "./types.ts";

const MONTH = /^\d{4}-(0[1-9]|1[0-2])$/;

class CliError extends Error {}

function read(file: string): string {
  try {
    return readFileSync(file, "utf8");
  } catch {
    throw new CliError(`error: cannot read ${file}`);
  }
}

function checkMonth(month: string): string {
  if (!MONTH.test(month)) throw new CliError("error: month must be YYYY-MM");
  return month;
}

function loadRules(file: string | undefined): Rule[] {
  if (file === undefined) return [];
  const [rules, bad] = parseRules(read(file));
  for (const line of bad) console.error(`warning: rules line ${line}: expected "category: keywords"`);
  return rules;
}

function run(argv: readonly string[]): string[] {
  const inv = parseArgs(argv);
  if ("error" in inv) throw new CliError(inv.error);
  const { command, positional, flags } = inv;
  const [file, arg] = positional as [string, string | undefined];

  const month = command === "report" ? checkMonth(arg!) : flags.month === undefined ? undefined : checkMonth(flags.month);
  const n = Number(arg);
  if (command === "top" && !(Number.isInteger(n) && n > 0)) throw new CliError("error: n must be a positive whole number");

  const { transactions, bad } = parseCsv(read(file));
  for (const b of bad) console.error(`warning: line ${b.line}: ${b.reason}`);

  switch (command) {
    case "list":
      return commands.list(transactions, bad.length);
    case "report":
      return commands.report(transactions, month!, loadRules(flags.rules));
    case "top":
      return commands.top(transactions, n, loadRules(flags.rules), month);
    case "months":
      return commands.months(transactions);
  }
}

try {
  for (const line of run(process.argv.slice(2))) console.log(line);
} catch (e) {
  if (!(e instanceof CliError)) throw e;
  console.error(e.message);
  process.exitCode = 1;
}

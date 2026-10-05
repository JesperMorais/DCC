# How we'd structure it

```
src/
  types.ts     the given data types
  csv.ts       one line → Row; whole file → good and bad rows
  rules.ts     rules file → Rule[]; description → category
  format.ts    cents → "1234.56"; one aligned report line
  commands.ts  list, report, top, months: transactions in, lines of text out
  args.ts      argv → command, positionals, flags (or an error)
  main.ts      the only file that reads files, prints and sets the exit code
```

## Decision 1: commands return lines, `main` does the I/O

Each command is a plain function from data to `string[]`:

```ts
export function report(txs: readonly Transaction[], month: string, rules: readonly Rule[]): string[]
```

It never touches `console` or `fs`, so it's easy to reason about (and to unit-test, should you want to). `main.ts` reads the files, prints warnings, calls a command and prints what comes back. Errors that should end the program (`cannot read`, bad month, bad `n`) are thrown as a small `CliError` class and turned into a message and exit code 1 in exactly one place. The trade-off: a throw is a hidden exit path. The alternative is to return a union like `Invocation | { error: string }`, which we do use for argument parsing, where the caller narrows with `"error" in result`. Both are fine; pick one style per layer.

## Decision 2: a bad row is a value, not an exception

`parseRow` returns `Row | null` (`null` for a blank line), and `Row` is `Transaction | BadRow`. Because only `BadRow` has `reason`, `"reason" in row` narrows it, and a type predicate lets `filter` split the rows into two correctly-typed arrays:

```ts
rows.filter((r): r is BadRow => "reason" in r)
```

Throwing on the first bad row would stop at one warning. Returning it as data lets the caller collect every problem, keep going and still say how many were skipped. The checks run in a fixed order with an early `return` each, which is what makes the warning text predictable.

## Decision 3: money is whole cents

`-89.90` becomes `-8990`. Sums of integers are exact, so the report's `TOTAL` always equals the sum of its lines, and `toFixed(2)` only runs at the very end, in `format.ts`. With floats, adding up a month of purchases can leave you at `1147.5499999…`, which prints fine until one day it doesn't.

## Smaller things worth noticing

- **Grouping:** a `Map<string, number>` of category → cents, then `[...map]` gives `[name, cents]` tuples to sort with two keys: `y - x || a.localeCompare(b)`. The `||` falls through to the tie-breaker only when the difference is 0.
- **Sorting safely:** `filter` already returns a new array, so sorting its result doesn't reorder the caller's data. Sorting the input directly would.
- **First match wins:** `rules.find(r => r.keywords.some(k => text.includes(k)))?.category ?? "other"`. Order matters, which is why rules are an array and not a `Record`.
- **Flags:** a `Record<Command, number>` of how many positionals each command takes keeps the "wrong number of arguments" check to one line, and adding a command forces you to add its entry.
- **Optional flags:** `Flags` has `rules?` and `month?`, so "not given" is `undefined`, not an empty string, and the compiler makes you handle it.

Where would it go next? Reading several CSV files at once, a `--csv` output mode for the report, or budgets per category in the rules file ("groceries: ica, coop | 3000") with a warning when a month goes over.

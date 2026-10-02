It's 02:40 and the checkout API is slow. The on-call engineer has ten thousand lines of access log and one question: *what is going on?* How many requests failed, which endpoint is dragging, and is it one user or everyone? Nobody reads ten thousand lines. They write a small analyser, and that's what you'll build in this boss. It uses every tool from the Core section, so this lesson is a map of which tool goes where.

### The input: text you don't trust

The log is one string, with one request per line:

```
09:15:03 GET /api/orders 200 120ms
09:15:04 POST /api/checkout 503 2400ms
```

Real logs have blank lines, half-written lines from a crash, and lines from some other process. A good analyser **never throws on bad input**. It counts what it couldn't read and moves on, so the on-call engineer knows how much to trust the numbers.

### Step 1: a type for one request (Interfaces, Unions)

Before parsing anything, decide what a parsed line *is*:

```ts
type Method = "GET" | "POST" | "PUT" | "DELETE";
interface LogRequest { time: string; method: Method; path: string; status: number; ms: number }
```

`Method` is a **literal union**, not `string`. Once a line has passed the parser, `request.method` can only be one of four values, and a typo like `"GTE"` in your own code is a compile error. A parser that returns `LogRequest | null` forces every caller to narrow with `if (request)` before using it.

### Step 2: text to `LogRequest` (regex, string parsing)

One anchored regex with capture groups does the whole line. Captures are always **strings**, so convert the numbers with `Number(...)`. The method capture is a `string` too, even if your regex only allows four words. The compiler can't read regexes, so you tell it with `as Method`, which is safe *because* the regex already checked it.

### Step 3: one pass, many answers (reduce, Records, Sets)

The report wants several things at once. You can loop once and update them all, or `reduce` into a typed accumulator. Pick the container for each answer:

| Question | Container |
|---|---|
| How many 2xx, 3xx, 4xx, 5xx? | `Record<StatusClass, number>`, a fixed key set where all four must exist |
| How many different endpoints? | `Set<string>`, then `.size` |
| Which paths to leave out? | `Set<string>` built from the options, for fast `has` |
| Slowest request? | a `[path, ms]` **tuple**, or `null` if there were none |

Turning `503` into `"5xx"` is a small function with a return type of `StatusClass`. Something like ``` `${Math.floor(status / 100)}xx` ``` works at runtime, but the compiler only sees a `string`. Spell out the cases, or cast after you've checked the range.

### Step 4: options with defaults (optional properties, `??`)

Health checks hit `/health` every second and drown everything else. The analyser takes an optional options object:

```ts
interface AnalyseOptions { ignorePaths?: readonly string[] }

function analyse(text: string, options: AnalyseOptions = {}) {
  const ignored = new Set(options.ignorePaths ?? []);
  …
}
```

`?` makes the property optional, and `??` supplies the default only when it's `undefined` or `null`.

### Step 5: format last (number formatting)

Keep `ms` as numbers all the way through and compute the average as a number. Only at the very end turn it into a display string with `toFixed(1)`. If you format early, `"120.0" + "80.0"` is `"120.080.0"`.

### Gotchas

- **Line numbers are 1-based** and count *every* line, including blank ones, because that's what an editor shows.
- **`text.split("\n")` keeps a final empty string** if the text ends with a newline. Trim each line, and treat empty ones as blank, not broken.
- **Ties**: "the slowest" with two equal durations should be the first one. Use `>` and not `>=` when you compare.
- **Don't divide by zero.** An empty log has no average, so decide what to show instead.

### In the wild

- **GoAccess, AWS CloudWatch Logs Insights and Datadog** do exactly this: parse lines, bucket by status class, find the slowest endpoints.
- **SRE dashboards** plot the 5xx count per minute. It's the first graph anyone opens during an incident.
- **`ignorePaths`** exists in nearly every real tool, because health checks and bots otherwise dominate the numbers.

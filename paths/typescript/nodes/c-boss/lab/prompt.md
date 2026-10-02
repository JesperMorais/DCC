It's 2 a.m., checkout is slow, and you have the API's access log. Build the analyser the on-call team will run. One request per line:

```
09:15:03 GET /api/orders 200 120ms
```

That's a time `HH:MM:SS`, a method (`GET`, `POST`, `PUT` or `DELETE`), a path starting with `/` (no spaces), a status from `200` to `599`, and a duration in whole milliseconds followed by `ms`. Single spaces between the fields. A line may have spaces around it.

**1. Make `Method` a union** of the four methods (the starter has `string`).

**2. `statusClass(status)`** returns `"2xx"`, `"3xx"`, `"4xx"` or `"5xx"` for a status from 200 to 599.

**3. `parseRequest(line)`** returns a `LogRequest` with `status` and `ms` as numbers, or `null` if the line doesn't match the format exactly.

**4. `analyse(text, options?)`** returns a `Report`:

| Field | Meaning |
|---|---|
| `requests` | how many requests were counted |
| `unreadable` | 1-based line numbers of non-blank lines that `parseRequest` rejects. Blank (or whitespace-only) lines are ignored, but they still count for numbering |
| `byStatus` | requests per status class. **All four keys** are present, even when 0 |
| `distinctPaths` | how many different paths were requested |
| `slowest` | `[path, ms]` of the slowest request (the first one on a tie), or `null` if no requests were counted |
| `averageMs` | the mean duration with **one decimal**, like `"1260.0"`, or `"n/a"` if no requests were counted |

`options.ignorePaths` is an optional list of paths (like `"/health"`). Requests to those paths are left out of every field. They're still readable lines, so they're not `unreadable`.

```ts
analyse("09:15:03 GET /api/orders 200 120ms\n\noops\n09:15:04 POST /api/checkout 503 2400ms");
// { requests: 2, unreadable: [3], byStatus: { "2xx": 1, "3xx": 0, "4xx": 0, "5xx": 1 },
//   distinctPaths: 2, slowest: ["/api/checkout", 2400], averageMs: "1260.0" }
```

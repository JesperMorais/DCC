Your site writes one line per page view:

```
09:15 ada /pricing
```

That's a time (`HH:MM`), a space, a username (lowercase letters `a–z`), a space, and a path that starts with `/` and has no spaces. Lines may have spaces around them. Anything else is junk from a crawler, and is skipped everywhere.

**1. `parseView(line)`** returns a `PageView` (`{ time, user, path }`), or `null` for a junk line.

**2. `viewsPerPage(lines)`** returns a `Record<string, number>`: how many views each path got. Paths with no views don't appear.

**3. `uniqueVisitors(lines)`** returns a `Map<string, number>`: for each path, how many **different** users viewed it. The map's keys are in the order each path was first seen.

```ts
const log = ["09:15 ada /pricing", "09:16 linus /docs", "09:20 ada /pricing", "GET /wp-admin"];
viewsPerPage(log);   // { "/pricing": 2, "/docs": 1 }
uniqueVisitors(log); // Map { "/pricing" => 1, "/docs" => 1 }
```

Don't modify `lines`.

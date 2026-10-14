**Bug report from a reader:** "I unsubscribed from your newsletter twice and I still get it every week. My address is Ana@Example.com."

`active_subscribers(signups, unsubscribes)` returns the addresses that should get the newsletter:

- Email addresses are **case-insensitive** and surrounding spaces are ignored, so `" Ana@Example.com"` and `"ana@example.com"` are the same reader.
- Anyone in `unsubscribes` is left out.
- Each reader appears **once**, the first time they signed up, written as they first typed it (with spaces stripped).

Examples:

- `active_subscribers(["bo@mail.se", "cy@mail.se"], ["cy@mail.se"])` → `["bo@mail.se"]`
- `active_subscribers(["Bo@mail.se", "bo@MAIL.se"], [])` → `["Bo@mail.se"]`
- `active_subscribers([], ["x@y.z"])` → `[]`

**Reproduction:** `active_subscribers(["Ana@Example.com", "bo@mail.se"], ["Ana@Example.com"])` returns `["Ana@Example.com", "bo@mail.se"]`, expected `["bo@mail.se"]`.

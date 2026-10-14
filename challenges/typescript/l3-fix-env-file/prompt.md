**Bug report from the backend team**

> After we moved to the new database, the app can't connect. The logs show it's trying to use a URL that just stops halfway, but the `.env` file looks perfect.

`parseEnvFile(text)` reads the contents of a `.env` file and returns an object of settings. Each line looks like `KEY=value`:

- The key is everything before the **first** `=`, the value is **everything after it**.
- Spaces around the key and the value are trimmed.
- Blank lines and lines starting with `#` (comments) are skipped.

```ts
parseEnvFile("PORT=8080\nDEBUG=true");
// { PORT: "8080", DEBUG: "true" }

parseEnvFile("# server\n\nHOST = localhost");
// { HOST: "localhost" }

parseEnvFile("");
// {}
```

**Reproduction:** `parseEnvFile("DB_URL=postgres://db/app?ssl=true")` returns `{ DB_URL: "postgres://db/app?ssl" }`, expected `{ DB_URL: "postgres://db/app?ssl=true" }`.

Find the bug and fix it.

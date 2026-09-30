# daily.ts

<img src="public/favicon.svg" width="72" align="right" alt="Tess the pangolin" />

One ~10-minute TypeScript challenge a day, on your own machine. The difficulty adapts to you.

```bash
npm install
npm start          # builds the UI and serves everything → http://localhost:4321
```

For hacking on the app itself: `npm run dev` (API on :4321 + Vite with hot reload on http://localhost:5173).

## How it works

- **46 challenges across 6 levels.** Level 1 is *First steps* (never coded) and level 6 is *Type wizardry* (conditional types, `infer`, template literal types). Each one has a task, a **Concept** tab (a short lesson on the idea it practises), 3 escalating hints and a reference solution.
- **Your code is really type-checked.** The server runs the TypeScript compiler in `strict` mode over your code *and* the tests together, so a wrong signature is a type error, not just a failing test. Then it runs the tests in a sandboxed worker thread, which kills infinite loops after 1.5s. Type-level challenges are judged by the compiler alone.
- **Adaptive difficulty (Elo).** You and every challenge have a rating. The daily pick sits slightly above your rating, avoids topics you just did and leans towards topics you're weak on. Solving pushes your rating up. Hints (−12% each), running well past 10 minutes and giving up pull it down. Only the first attempt at a challenge is rated; after that it's free practice.
- **Dashboard.** Rating over time, streaks, a 20-week activity heatmap, topic mastery, level progress and recent attempts.
- **Tess the pangolin** is the mascot. A pangolin's scales are armour, and so are types. Tess reacts to how your day is going (`src/components/Mascot.tsx`, all moods at `/mascot`).
- **Everything local.** Progress lives in `data/progress.json`. The Monaco editor is bundled, so it works offline.

## Adding challenges

See [`challenges/README.md`](challenges/README.md). A challenge is just a folder. Check it with `npm run validate`, which makes sure the reference solution passes, the starter fails and the rating fits its level.

## Layout

```
server/            Hono API, adaptive engine (engine.ts), JSON store
server/runner/     type-check (TS compiler API) + sandboxed test execution
challenges/        the curriculum (one folder per challenge)
src/               React + Tailwind UI (Dashboard, Challenge workspace, Library, Settings)
```

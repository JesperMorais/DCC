# daily.ts

Daily ~10-minute coding challenges in **TypeScript, Python and C** that adapt to your level. Everything runs locally.

<img src="public/favicon.svg" width="72" align="right" alt="Tess the pangolin" />


```bash
npm install
npm start          # builds the UI and serves everything → http://localhost:4321
```

For hacking on the app itself: `npm run dev` (API on :4321 + Vite with hot reload on http://localhost:5173).

## How it works

- **Three tracks with about 45 challenges each, over 6 levels.** Every challenge has a task, a **Concept** tab (a short lesson), 3 escalating hints and a reference solution.
  - **TypeScript:** type-checked in strict mode together with the tests, and executed in a sandboxed worker. Includes type-level challenges.
  - **Python:** pytest-style tests with plain `assert`, rewritten so failures show expected vs received. Each test has a timeout, tracebacks point at your line, and `mypy --strict` feedback on your type hints appears as warnings.
  - **C:** `gcc -std=c17 -Wall -Wextra -pedantic` with **AddressSanitizer + UBSan** and a leak check. Each test runs in its own process, so a segfault, an out-of-bounds read, a leak or signed overflow fails that test with a plain-English explanation of the line it happened on.
- **Adaptive difficulty (Elo), one rating per language.**
  - **Daily pick:** slightly above your rating, and it leans towards the topics you're weakest at.
  - **What moves the rating:** solving raises it. Hints scale the gain down (a hard solve that needed every hint leaves it unchanged), and giving up pulls it down. Time doesn't count. Only your first attempt at a challenge is rated.
  - **Dailies → skill tree:** a daily that went badly (gave up, ≥2 hints, or well over the estimate) marks the tree nodes that teach its topics as "needs practice". It's a recommendation, never a block.
  - **Streaks:** one streak across all languages.
- **Tess the pangolin** is the mascot. A pangolin's scales are armour, and so are types. Tess greets you, reacts to your runs and suggests the free Concept lesson when you're stuck. You can set Tess to Chatty, Quiet or Off in Settings.
- **Dashboard (per language):** your rating over time, a 20-week activity heatmap, topic mastery, level progress and recent attempts.
- **Everything local.** Progress lives in `data/progress.json`, and older single-language progress files are migrated automatically, with a backup. The Monaco editor is bundled, so it works offline.

### Requirements

- Node 20+
- For the Python track: `python3` 3.12+ (optionally `mypy`, for type-hint feedback)
- For the C track: `gcc` with AddressSanitizer (standard on Linux; on macOS set `DAILY_TS_CC=clang`)

## Adding challenges

See [`challenges/README.md`](challenges/README.md). A challenge is just a folder. Check it with `npm run validate`, which makes sure the reference solution passes, the starter fails and the rating fits its level.

## Layout

```
server/            Hono API, adaptive engine (engine.ts), JSON store
server/runner/     per-language runners: TS (compiler API + worker), Python (harness.py), C (gcc + ASan harness)
shared/            language registry used by server and UI
challenges/        the curriculum (one folder per challenge)
src/               React + Tailwind UI (Dashboard, Challenge workspace, Library, Settings)
```

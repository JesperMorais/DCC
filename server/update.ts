import { execFile } from "node:child_process";
import path from "node:path";
import { fileURLToPath } from "node:url";

/**
 * Self-update from the git remote. The app is a git checkout, so "is there an update" is
 * "is the upstream branch ahead of HEAD", and updating is a fast-forward pull. Anything else
 * (local edits, local commits, no upstream, offline) is reported and left to the terminal.
 */

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const BIN = path.join(ROOT, "node_modules", ".bin");
/** Exit code that tells scripts/launch.ts to start the server again. */
export const RESTART_CODE = 75;

function run(cmd: string, args: string[], timeout = 20_000) {
  return new Promise<string>((resolve, reject) =>
    execFile(
      cmd,
      args,
      // Never wait for a username/password prompt: a private repo without saved credentials just counts as offline.
      { cwd: ROOT, timeout, maxBuffer: 16 * 1024 * 1024, env: { ...process.env, GIT_TERMINAL_PROMPT: "0" } },
      (err, stdout, stderr) => (err ? reject(new Error((stderr || err.message).trim())) : resolve(stdout.trim())),
    ),
  );
}
const git = (...args: string[]) => run("git", args);

export type UpdateStatus =
  | { state: "unavailable"; reason: string }
  | { state: "offline" }
  | { state: "up-to-date"; commit: string; checkedAt: string }
  | {
      state: "available";
      commit: string;
      checkedAt: string;
      behind: number;
      commits: { hash: string; subject: string; date: string }[];
      /** Why the button can't do it, if it can't. The update then has to happen in a terminal. */
      blocked: string | null;
    };

let cached: { at: number; status: UpdateStatus } | null = null;
const CACHE_MS = 10 * 60_000;
let updating = false;

export const currentCommit = () => git("rev-parse", "--short", "HEAD").catch(() => null);

export async function checkForUpdate(force = false): Promise<UpdateStatus> {
  if (!force && cached && Date.now() - cached.at < CACHE_MS) return cached.status;
  const status = await check();
  cached = { at: Date.now(), status };
  return status;
}

async function check(): Promise<UpdateStatus> {
  let upstream: string;
  try {
    await git("rev-parse", "--is-inside-work-tree");
  } catch {
    return { state: "unavailable", reason: "This copy isn't a git checkout, so it can't update itself. Download a fresh copy instead." };
  }
  try {
    upstream = await git("rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}");
  } catch {
    return { state: "unavailable", reason: "This branch doesn't track a remote branch." };
  }
  try {
    await run("git", ["fetch", "--quiet", upstream.split("/")[0]], 15_000);
  } catch {
    return { state: "offline" };
  }
  const commit = await git("rev-parse", "--short", "HEAD");
  const checkedAt = new Date().toISOString();
  const [ahead, behind] = (await git("rev-list", "--left-right", "--count", "HEAD...@{u}")).split(/\s+/).map(Number);
  if (behind === 0) return { state: "up-to-date", commit, checkedAt };

  const log = await git("log", "--format=%h%x09%cI%x09%s", "HEAD..@{u}");
  const commits = log
    .split("\n")
    .filter(Boolean)
    .map((l) => {
      const [hash, date, ...rest] = l.split("\t");
      return { hash, date, subject: rest.join("\t") };
    });
  // Untracked files are fine (a pull refuses on its own if one would be overwritten); edits to tracked files are not.
  const dirty = (await git("status", "--porcelain", "--untracked-files=no")) !== "";
  const blocked =
    ahead > 0
      ? `This copy has ${ahead} commit${ahead > 1 ? "s" : ""} that ${upstream} doesn't, so it can't just fast-forward. Update with git in a terminal.`
      : dirty
        ? "This copy has local changes to the app's files. Commit or stash them, then update (or run git pull in a terminal)."
        : null;
  return { state: "available", commit, checkedAt, behind, commits, blocked };
}

/**
 * `then`: "restart" = the server exits and the launcher starts the new version; "reload" = dev mode,
 * where tsx watch and Vite pick up the changes on their own; "manual" = no launcher, restart by hand.
 */
export type UpdateResult = { ok: true; from: string; to: string; then: "restart" | "reload" | "manual"; note: string | null } | { ok: false; error: string };

/**
 * Fast-forward to the upstream, reinstall dependencies if they changed and rebuild the UI if
 * it changed. In production the caller then exits with RESTART_CODE and the launcher restarts
 * the server. In dev, `tsx watch` and Vite pick the changes up by themselves.
 */
export async function applyUpdate(): Promise<UpdateResult> {
  if (updating) return { ok: false, error: "An update is already running." };
  updating = true;
  try {
    const status = await checkForUpdate(true);
    if (status.state !== "available") return { ok: false, error: status.state === "offline" ? "Couldn't reach the repository." : "There's nothing to update." };
    if (status.blocked) return { ok: false, error: status.blocked };

    const from = await git("rev-parse", "HEAD");
    await run("git", ["pull", "--ff-only", "--quiet"], 60_000);
    const to = await git("rev-parse", "HEAD");
    const changed = (await git("diff", "--name-only", from, to)).split("\n");

    if (changed.some((f) => f === "package.json" || f === "package-lock.json")) await run("npm", ["install", "--no-audit", "--no-fund"], 5 * 60_000);

    const production = process.env.NODE_ENV === "production";
    const uiChanged = changed.some((f) => /^(src|public|shared)\/|^(index\.html|vite\.config\.ts|package(-lock)?\.json|tsconfig\.json)$/.test(f));
    if (production && uiChanged) await run(path.join(BIN, "vite"), ["build"], 5 * 60_000);

    const launched = process.env.DAILY_TS_LAUNCHER === "1";
    cached = null;
    return {
      ok: true,
      from: from.slice(0, 7),
      to: to.slice(0, 7),
      then: !production ? "reload" : launched ? "restart" : "manual",
      note:
        production && !launched
          ? "Updated. Stop the server and run npm start again to use the new version."
          : !production && changed.some((f) => f.startsWith("package"))
            ? "Dependencies changed. If something looks off, restart npm run dev."
            : null,
    };
  } catch (e) {
    return { ok: false, error: (e as Error).message };
  } finally {
    updating = false;
  }
}

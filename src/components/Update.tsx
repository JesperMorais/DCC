import { useEffect, useState, useSyncExternalStore } from "react";
import { api, type UpdateResult, type UpdateStatus } from "../api";
import { Icon } from "../ui";

/* One shared update status for the sidebar and Settings. Checked on load and every 6 hours. */
let status: UpdateStatus | null = null;
let checking = false;
const listeners = new Set<() => void>();
const emit = () => listeners.forEach((l) => l());

export async function checkForUpdate(force = false) {
  if (checking) return;
  checking = true;
  emit();
  try {
    status = await api.checkUpdate(force);
  } catch {
    status = { state: "offline" };
  } finally {
    checking = false;
    emit();
  }
}

function useUpdate() {
  return useSyncExternalStore(
    (l) => (listeners.add(l), () => listeners.delete(l)),
    () => status,
  );
}
const useChecking = () =>
  useSyncExternalStore(
    (l) => (listeners.add(l), () => listeners.delete(l)),
    () => checking,
  );

let polling = false;
function startPolling() {
  if (polling) return;
  polling = true;
  checkForUpdate();
  setInterval(() => checkForUpdate(), 6 * 3_600_000);
}

const sleep = (ms: number) => new Promise((r) => setTimeout(r, ms));

/** After an update the server restarts. Wait until it answers with the new commit, then reload the page. */
async function reloadWhenBack(commit: string) {
  const until = Date.now() + 3 * 60_000;
  while (Date.now() < until) {
    await sleep(1000);
    try {
      if ((await api.version()).commit === commit) return location.reload();
    } catch {
      // still restarting
    }
  }
  throw new Error("The server didn't come back. Check the terminal, then reload the page.");
}

/** Sidebar entry: only there when an update is available. */
export function SidebarUpdate() {
  const s = useUpdate();
  const [open, setOpen] = useState(false);
  useEffect(startPolling, []);
  if (s?.state !== "available") return null;
  return (
    <>
      <button
        className="flex items-center gap-3 rounded-lg border border-accent/40 bg-accent-soft px-2.5 py-2 text-sm font-medium text-accent-strong hover:border-accent"
        onClick={() => setOpen(true)}
        title="An update is available"
      >
        <Icon name="down" size={18} />
        <span className="hidden lg:block">Update available</span>
      </button>
      {open && <UpdateDialog status={s} onClose={() => setOpen(false)} />}
    </>
  );
}

/** Settings section: current version, last check, and a manual check. */
export function UpdateSettings() {
  const s = useUpdate();
  const busy = useChecking();
  const [open, setOpen] = useState(false);
  const [commit, setCommit] = useState<string | null>(null);
  useEffect(() => {
    startPolling();
    api.version().then((v) => setCommit(v.commit), () => {});
  }, []);
  const text = !s
    ? "Checking…"
    : s.state === "unavailable"
      ? s.reason
      : s.state === "offline"
        ? "Couldn't reach the repository. You're offline, or it needs credentials this machine doesn't have."
        : s.state === "up-to-date"
          ? `You're on the latest version. Checked ${new Date(s.checkedAt).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" })}.`
          : `${s.behind} new change${s.behind > 1 ? "s" : ""} available.`;
  return (
    <section className="card mt-5 p-5">
      <div className="flex items-start justify-between gap-4">
        <div>
          <h2 className="text-[15px] font-semibold">Updates</h2>
          <p className="mt-1 text-sm text-ink-2">{text}</p>
          {commit && <p className="mt-1 text-xs text-muted">Version {commit}</p>}
        </div>
        {s?.state === "available" ? (
          <button className="btn btn-primary shrink-0" onClick={() => setOpen(true)}>
            See what's new
          </button>
        ) : (
          s?.state !== "unavailable" && (
            <button className="btn shrink-0" onClick={() => checkForUpdate(true)} disabled={busy}>
              <Icon name="refresh" size={14} /> {busy ? "Checking…" : "Check now"}
            </button>
          )
        )}
      </div>
      {open && s?.state === "available" && <UpdateDialog status={s} onClose={() => setOpen(false)} />}
    </section>
  );
}

type Phase = { step: "idle" } | { step: "updating" } | { step: "restarting" } | { step: "done"; note: string } | { step: "error"; error: string };

function UpdateDialog({ status, onClose }: { status: Extract<UpdateStatus, { state: "available" }>; onClose: () => void }) {
  const [phase, setPhase] = useState<Phase>({ step: "idle" });
  const busy = phase.step === "updating" || phase.step === "restarting";

  useEffect(() => {
    const k = (e: KeyboardEvent) => e.key === "Escape" && !busy && onClose();
    window.addEventListener("keydown", k);
    return () => window.removeEventListener("keydown", k);
  }, [onClose, busy]);

  const update = async () => {
    setPhase({ step: "updating" });
    const r: UpdateResult = await api.applyUpdate();
    if (!r.ok) return setPhase({ step: "error", error: r.error });
    if (r.then === "manual") return setPhase({ step: "done", note: r.note ?? "Updated. Restart the app to use it." });
    setPhase({ step: "restarting" });
    try {
      // Dev mode: HEAD is already new, so give tsx watch a moment to restart the API first.
      if (r.then === "reload") await sleep(2500);
      await reloadWhenBack(r.to);
    } catch (e) {
      setPhase({ step: "error", error: (e as Error).message });
    }
  };

  return (
    <div className="fixed inset-0 z-50 grid place-items-center bg-black/50 p-4 backdrop-blur-sm" onClick={() => !busy && onClose()}>
      <div className="card pop-in max-h-[90vh] w-full max-w-lg overflow-y-auto p-6" onClick={(e) => e.stopPropagation()}>
        <h2 className="text-lg font-semibold">
          {status.behind} new change{status.behind > 1 ? "s" : ""}
        </h2>
        <p className="mt-1 text-sm text-ink-2">Your progress is stored separately and isn't touched by updates.</p>
        <ul className="mt-4 flex max-h-64 flex-col gap-1.5 overflow-y-auto rounded-lg border border-line bg-surface-2 p-3 text-sm">
          {status.commits.map((c) => (
            <li key={c.hash} className="flex gap-2">
              <span className="shrink-0 text-xs text-muted tabular">{new Date(c.date).toLocaleDateString()}</span>
              <span className="min-w-0">{c.subject}</span>
            </li>
          ))}
        </ul>

        {status.blocked && <p className="mt-4 rounded-lg border border-warn/40 bg-warn-soft px-3 py-2 text-sm text-warn">{status.blocked}</p>}
        {phase.step === "updating" && <p className="mt-4 text-sm text-ink-2">Downloading and installing… this can take a minute if dependencies changed.</p>}
        {phase.step === "restarting" && <p className="mt-4 text-sm text-ink-2">Restarting the app. The page reloads by itself.</p>}
        {phase.step === "done" && <p className="mt-4 text-sm text-ink-2">{phase.note}</p>}
        {phase.step === "error" && <p className="mt-4 rounded-lg border border-bad/40 px-3 py-2 text-sm text-bad">{phase.error}</p>}

        <div className="mt-6 flex justify-end gap-2">
          <button className="btn" onClick={onClose} disabled={busy}>
            {phase.step === "done" ? "Close" : "Later"}
          </button>
          {phase.step !== "done" && (
            <button className="btn btn-primary" onClick={update} disabled={busy || !!status.blocked}>
              <Icon name="down" size={14} /> {busy ? "Updating…" : "Update"}
            </button>
          )}
        </div>
      </div>
    </div>
  );
}

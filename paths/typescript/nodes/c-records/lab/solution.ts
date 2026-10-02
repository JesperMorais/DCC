interface PageView {
  time: string;
  user: string;
  path: string;
}

function parseView(line: string): PageView | null {
  const m = line.trim().match(/^(\d\d:\d\d) ([a-z]+) (\/\S*)$/);
  if (!m) return null;
  const [, time, user, path] = m;
  return { time, user, path };
}

function viewsPerPage(lines: readonly string[]): Record<string, number> {
  const views: Record<string, number> = {};
  for (const line of lines) {
    const view = parseView(line);
    if (view) views[view.path] = (views[view.path] ?? 0) + 1;
  }
  return views;
}

function uniqueVisitors(lines: readonly string[]): Map<string, number> {
  const users = new Map<string, Set<string>>();
  for (const line of lines) {
    const view = parseView(line);
    if (!view) continue;
    const seen = users.get(view.path) ?? new Set<string>();
    seen.add(view.user);
    users.set(view.path, seen);
  }
  return new Map([...users].map(([path, seen]) => [path, seen.size]));
}

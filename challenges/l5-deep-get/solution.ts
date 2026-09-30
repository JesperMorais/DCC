function getPath(obj: unknown, path: string): unknown {
  if (path === "") return obj;
  if (typeof obj !== "object" || obj === null) return undefined;
  const dot = path.indexOf(".");
  const head = dot === -1 ? path : path.slice(0, dot);
  const rest = dot === -1 ? "" : path.slice(dot + 1);
  return getPath((obj as Record<string, unknown>)[head], rest);
}

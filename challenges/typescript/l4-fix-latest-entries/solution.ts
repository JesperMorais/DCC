function latestEntries<T extends { at: number }>(entries: readonly T[], n: number): T[] {
  const sorted = [...entries].sort((a, b) => a.at - b.at);
  return n > 0 ? sorted.slice(-n) : [];
}

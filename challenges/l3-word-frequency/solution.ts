function wordFrequency(text: string): Record<string, number> {
  const counts: Record<string, number> = {};
  const words = text.toLowerCase().match(/[a-z0-9]+/g) ?? [];
  for (const word of words) {
    counts[word] = (counts[word] ?? 0) + 1;
  }
  return counts;
}

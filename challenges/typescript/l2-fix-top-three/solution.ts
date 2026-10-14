function topThree(scores: number[]): number[] {
  const sorted = [...scores].sort((a, b) => b - a);
  return sorted.slice(0, 3);
}

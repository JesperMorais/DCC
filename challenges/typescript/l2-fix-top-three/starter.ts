function topThree(scores: number[]): number[] {
  const sorted = [...scores].sort().reverse();
  return sorted.slice(0, 3);
}

function everyonePassed(scores: number[], passMark: number): boolean {
  for (const score of scores) {
    if (score < passMark) {
      return false;
    }
  }
  return true;
}

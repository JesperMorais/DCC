function averageSteps(days: number[]): number {
  if (days.length === 0) return 0;
  let total = 0;
  for (const n of days) {
    total += n;
  }
  return Math.round(total / days.length);
}

function improvedDays(days: number[]): number {
  let count = 0;
  for (let i = 1; i < days.length; i++) {
    if (days[i] > days[i - 1]) count++;
  }
  return count;
}

function longestStreak(days: number[], goal: number): number {
  let current = 0;
  let best = 0;
  for (const n of days) {
    if (n >= goal) {
      current++;
      if (current > best) best = current;
    } else {
      current = 0;
    }
  }
  return best;
}

interface Run {
  day: string;
  km: number;
}

function weeklyKm(runs: Run[]): number {
  const total = runs.map((run) => run.km).reduce((sum, km) => sum + km, 0);
  return Math.round(total * 10) / 10;
}

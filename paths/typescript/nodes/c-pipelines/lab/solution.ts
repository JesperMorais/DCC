interface Run {
  runner: string;
  seconds: number;
  finished: boolean;
}

interface RaceStats {
  finishers: number;
  dnf: number;
  average: number | null;
}

function podium(runs: readonly Run[]): string[] {
  return runs
    .filter((run) => run.finished)
    .sort((a, b) => a.seconds - b.seconds)
    .slice(0, 3)
    .map((run, i) => `${i + 1}. ${run.runner} ${run.seconds.toFixed(2)}s`);
}

interface Tally {
  finishers: number;
  dnf: number;
  total: number;
}

function raceStats(runs: readonly Run[]): RaceStats {
  const tally = runs.reduce<Tally>(
    (acc, run) =>
      run.finished
        ? { ...acc, finishers: acc.finishers + 1, total: acc.total + run.seconds }
        : { ...acc, dnf: acc.dnf + 1 },
    { finishers: 0, dnf: 0, total: 0 },
  );
  return {
    finishers: tally.finishers,
    dnf: tally.dnf,
    average: tally.finishers === 0 ? null : tally.total / tally.finishers,
  };
}

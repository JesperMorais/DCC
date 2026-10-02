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

function podium(runs: any): string[] {
  return [];
}

function raceStats(runs: any): RaceStats {
  return { finishers: 0, dnf: 0, average: null };
}

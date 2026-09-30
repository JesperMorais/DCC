function minMax(temps: readonly number[]): [min: number, max: number] | undefined {
  if (temps.length === 0) return undefined;
  return [Math.min(...temps), Math.max(...temps)];
}

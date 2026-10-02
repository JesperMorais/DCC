function newest<T extends { createdAt: number }>(items: readonly T[]): T | undefined {
  let best: T | undefined;
  for (const item of items) {
    if (best === undefined || item.createdAt > best.createdAt) best = item;
  }
  return best;
}

function findBy<T, K extends keyof T>(items: readonly T[], key: K, value: T[K]): T | undefined {
  return items.find((item) => item[key] === value);
}

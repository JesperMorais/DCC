function pushRecent<T>(list: readonly T[], item: T, max: number): T[] {
  return [item, ...list.filter((x) => x !== item)].slice(0, max);
}

function zip<A, B>(as: readonly A[], bs: readonly B[]): [A, B][] {
  const pairs: [A, B][] = [];
  for (let i = 0; i < Math.min(as.length, bs.length); i++) {
    pairs.push([as[i], bs[i]]);
  }
  return pairs;
}

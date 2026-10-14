function isPresent<T>(value: T | null | undefined): value is T {
  return value !== null && value !== undefined;
}

function findByIds<K, V>(records: ReadonlyMap<K, V>, ids: readonly K[]): V[] {
  return ids.map((id) => records.get(id)).filter(isPresent);
}

function patchItemAt<T>(list: readonly T[], index: number, patch: Partial<T>): T[] {
  const next = [...list];
  if (index < 0 || index >= list.length) return [...list];
  next[index] = { ...list[index], ...patch };
  return next;
}

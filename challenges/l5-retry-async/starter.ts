async function retry<T>(fn: () => Promise<T>, attempts: number): Promise<T> {
  // TODO: retry on failure
  return fn();
}

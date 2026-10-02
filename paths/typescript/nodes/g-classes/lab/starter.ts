class PoolExhaustedError extends Error {}

class Pool<T> {
  constructor(resources: readonly T[]) {}

  get available(): number {
    return 0;
  }

  acquire(): any {
    return undefined;
  }

  release(resource: T): void {}

  use(fn: (resource: T) => any): any {
    return undefined;
  }
}

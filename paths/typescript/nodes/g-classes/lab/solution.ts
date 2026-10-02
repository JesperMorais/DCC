class PoolExhaustedError extends Error {
  readonly capacity: number;

  constructor(capacity: number) {
    super(`All ${capacity} resources are in use`);
    this.name = "PoolExhaustedError";
    this.capacity = capacity;
  }
}

class Pool<T> {
  #free: T[];
  #inUse: T[] = [];
  #capacity: number;

  constructor(resources: readonly T[]) {
    this.#free = [...resources];
    this.#capacity = resources.length;
  }

  get available(): number {
    return this.#free.length;
  }

  acquire(): T {
    if (this.#free.length === 0) throw new PoolExhaustedError(this.#capacity);
    const resource = this.#free.shift() as T;
    this.#inUse.push(resource);
    return resource;
  }

  release(resource: T): void {
    const index = this.#inUse.indexOf(resource);
    if (index === -1) throw new Error("Resource is not checked out");
    this.#inUse.splice(index, 1);
    this.#free.push(resource);
  }

  use<R>(fn: (resource: T) => R): R {
    const resource = this.acquire();
    try {
      return fn(resource);
    } finally {
      this.release(resource);
    }
  }
}

class Stack<T> {
  push(item: T): void {}

  pop(): T {
    throw new Error("not implemented");
  }

  peek(): T | undefined {
    return undefined;
  }

  get size(): number {
    return 0;
  }
}

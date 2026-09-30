class Stack<T> {
  #items: T[] = [];

  push(item: T): void {
    this.#items.push(item);
  }

  pop(): T {
    if (this.#items.length === 0) throw new Error("Stack is empty");
    return this.#items.pop()!; // `!`: we just checked it isn't empty
  }

  peek(): T | undefined {
    return this.#items[this.#items.length - 1];
  }

  get size(): number {
    return this.#items.length;
  }
}

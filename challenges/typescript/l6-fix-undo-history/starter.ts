class UndoHistory<T extends object> {
  private past: T[] = [];
  private future: T[] = [];
  private current: T;

  constructor(
    initial: T,
    private readonly limit = 100,
  ) {
    this.current = structuredClone(initial);
  }

  get present(): T {
    return this.current;
  }

  get canUndo(): boolean {
    return this.past.length > 0;
  }

  get canRedo(): boolean {
    return this.future.length > 0;
  }

  commit(recipe: (draft: T) => void): void {
    const draft = { ...this.current };
    recipe(draft);
    this.past.push(this.current);
    if (this.past.length > this.limit) this.past.shift();
    this.current = draft;
    this.future = [];
  }

  undo(): boolean {
    const previous = this.past.pop();
    if (previous === undefined) return false;
    this.future.push(this.current);
    this.current = previous;
    return true;
  }

  redo(): boolean {
    const next = this.future.pop();
    if (next === undefined) return false;
    this.past.push(this.current);
    this.current = next;
    return true;
  }
}

import { EventBus, type Listener } from "./bus.ts";

export type Reducer<S, A> = (state: S, action: A) => S;

/** What caused a change. Tagged by `kind`, so listeners can narrow on it. */
export type Cause<S, A> =
  | { kind: "action"; action: A }
  | { kind: "patch"; patch: Partial<S> }
  | { kind: "undo" };

export interface StoreEvents<S, A> {
  change: { prev: Readonly<S>; next: Readonly<S>; cause: Cause<S, A> };
  rejected: { action: A; error: InvalidActionError };
}

/** Thrown by reducers for actions that make no sense in the current state. */
export class InvalidActionError extends Error {
  readonly action: { type: string };

  constructor(action: { type: string }, reason: string) {
    super(`${action.type}: ${reason}`);
    this.name = "InvalidActionError";
    this.action = action;
  }
}

export class Store<S extends object, A extends { type: string }> {
  #state: S;
  #reducer: Reducer<S, A>;
  #history: S[] = [];
  #bus = new EventBus<StoreEvents<S, A>>();

  constructor(initial: S, reducer: Reducer<S, A>) {
    this.#state = initial;
    this.#reducer = reducer;
  }

  get state(): Readonly<S> {
    return this.#state;
  }

  get canUndo(): boolean {
    return this.#history.length > 0;
  }

  select<K extends keyof S>(key: K): S[K] {
    return this.#state[key];
  }

  /** Runs the reducer. Returns false (and emits "rejected") for an InvalidActionError; other errors are bugs and propagate. */
  dispatch(action: A): boolean {
    let next: S;
    try {
      next = this.#reducer(this.#state, action);
    } catch (error) {
      if (!(error instanceof InvalidActionError)) throw error;
      this.#bus.emit("rejected", { action, error });
      return false;
    }
    this.#commit(next, { kind: "action", action });
    return true;
  }

  update(patch: Partial<S>): void {
    const keys = Object.keys(patch) as (keyof S)[];
    if (!keys.some((k) => patch[k] !== this.#state[k])) return;
    this.#commit({ ...this.#state, ...patch }, { kind: "patch", patch });
  }

  undo(): boolean {
    const prev = this.#history.pop();
    if (prev === undefined) return false;
    this.#set(prev, { kind: "undo" });
    return true;
  }

  /** The milestone-1 API, kept so its tests still pass: now just a view on the change event. */
  subscribe(listener: (state: Readonly<S>) => void): () => void {
    return this.on("change", ({ next }) => listener(next));
  }

  on<K extends keyof StoreEvents<S, A>>(event: K, listener: Listener<StoreEvents<S, A>[K]>): () => void {
    return this.#bus.on(event, listener);
  }

  /** Calls `listener` only when `state[key]` changes (by ===). */
  watch<K extends keyof S>(key: K, listener: (next: S[K], prev: S[K]) => void): () => void {
    return this.on("change", ({ prev, next }) => {
      if (next[key] !== prev[key]) listener(next[key], prev[key]);
    });
  }

  #commit(next: S, cause: Cause<S, A>): void {
    if (next === this.#state) return; // the reducer said "nothing to do"
    this.#history.push(this.#state);
    this.#set(next, cause);
  }

  #set(next: S, cause: Cause<S, A>): void {
    const prev = this.#state;
    this.#state = next;
    this.#bus.emit("change", { prev, next, cause });
  }
}

interface PlayerState {
  track: string | null;
  status: "stopped" | "playing" | "paused";
  volume: number;
  shuffle: boolean;
}

type PlayerAction =
  | { type: "play"; track: string }
  | { type: "pause" }
  | { type: "stop" }
  | { type: "setVolume"; volume: number };

const initialPlayer: PlayerState = { track: null, status: "stopped", volume: 50, shuffle: false };

function assertNever(value: never): never {
  throw new Error(`Unexpected value: ${JSON.stringify(value)}`);
}

class InvalidActionError extends Error {
  readonly action: { type: string };

  constructor(action: { type: string }, reason: string) {
    super(`${action.type}: ${reason}`);
    this.name = "InvalidActionError";
    this.action = action;
  }
}

function playerReducer(state: PlayerState, action: PlayerAction): PlayerState {
  switch (action.type) {
    case "play":
      return { ...state, track: action.track, status: "playing" };
    case "pause":
      if (state.status !== "playing") throw new InvalidActionError(action, "Nothing is playing");
      return { ...state, status: "paused" };
    case "stop":
      return { ...state, track: null, status: "stopped" };
    case "setVolume":
      if (!Number.isInteger(action.volume) || action.volume < 0 || action.volume > 100) {
        throw new InvalidActionError(action, "Volume must be 0-100");
      }
      return { ...state, volume: action.volume };
    default:
      return assertNever(action);
  }
}

class Store<S extends object, A extends { type: string }> {
  #state: S;
  #reducer: (state: S, action: A) => S;
  #listeners: ((state: Readonly<S>) => void)[] = [];

  constructor(initial: S, reducer: (state: S, action: A) => S) {
    this.#state = initial;
    this.#reducer = reducer;
  }

  get state(): Readonly<S> {
    return this.#state;
  }

  select<K extends keyof S>(key: K): S[K] {
    return this.#state[key];
  }

  dispatch(action: A): void {
    this.#set(this.#reducer(this.#state, action));
  }

  update(patch: Partial<S>): void {
    this.#set({ ...this.#state, ...patch });
  }

  subscribe(listener: (state: Readonly<S>) => void): () => void {
    this.#listeners.push(listener);
    return () => {
      this.#listeners = this.#listeners.filter((l) => l !== listener);
    };
  }

  #set(next: S): void {
    this.#state = next;
    for (const listener of this.#listeners) listener(next);
  }
}

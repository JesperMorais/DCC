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

class InvalidActionError extends Error {}

function playerReducer(state: PlayerState, action: PlayerAction): PlayerState {
  return state;
}

class Store {
  constructor(initial: any, reducer: any) {}

  get state(): any {
    return undefined;
  }

  select(key: any): any {
    return undefined;
  }

  dispatch(action: any): void {}

  update(patch: any): void {}

  subscribe(listener: any): () => void {
    return () => {};
  }
}

// The music player from the Generics boss lab, copied over as the first domain for the store.
import { InvalidActionError } from "../src/store.ts";

export interface PlayerState {
  track: string | null;
  status: "stopped" | "playing" | "paused";
  volume: number;
  shuffle: boolean;
}

export type PlayerAction =
  | { type: "play"; track: string }
  | { type: "pause" }
  | { type: "stop" }
  | { type: "setVolume"; volume: number };

export const initialPlayer: PlayerState = { track: null, status: "stopped", volume: 50, shuffle: false };

function assertNever(value: never): never {
  throw new Error(`Unexpected value: ${JSON.stringify(value)}`);
}

export function playerReducer(state: PlayerState, action: PlayerAction): PlayerState {
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

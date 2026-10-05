// A tiny domain for testing the generic store, unrelated to the cart on purpose.
import { InvalidActionError } from "../src/store.ts";

export interface Counter {
  count: number;
  step: number;
  label: string;
}

export type CounterAction = { type: "increment" } | { type: "reset" } | { type: "setStep"; step: number } | { type: "noop" } | { type: "crash" };

export const initial: Counter = { count: 0, step: 1, label: "clicks" };

export function counter(state: Counter, action: CounterAction): Counter {
  switch (action.type) {
    case "increment":
      return { ...state, count: state.count + state.step };
    case "reset":
      return { ...state, count: 0 };
    case "setStep":
      if (action.step <= 0) throw new InvalidActionError("step must be positive");
      return { ...state, step: action.step };
    case "noop":
      return state;
    case "crash":
      throw new TypeError("a bug in the reducer");
  }
}

// The boss lab's checks, ported from expect(...) to node:assert.
import { test } from "node:test";
import assert from "node:assert/strict";
import { InvalidActionError, Store } from "../src/store.ts";
import { initialPlayer, playerReducer, type PlayerAction, type PlayerState } from "./player.ts";
import type { Equal, Expect } from "./type-helpers.ts";

test("play, pause and stop move through the states", () => {
  const player = new Store(initialPlayer, playerReducer);
  player.dispatch({ type: "play", track: "Dancing Queen" });
  assert.deepEqual(player.state, { track: "Dancing Queen", status: "playing", volume: 50, shuffle: false });
  player.dispatch({ type: "pause" });
  assert.equal(player.select("status"), "paused");
  player.dispatch({ type: "stop" });
  assert.deepEqual(player.state, { track: null, status: "stopped", volume: 50, shuffle: false });
});

test("the reducer throws InvalidActionError for actions that make no sense", () => {
  assert.throws(() => playerReducer(initialPlayer, { type: "pause" }), { name: "InvalidActionError", message: "pause: Nothing is playing" });
  assert.throws(() => playerReducer(initialPlayer, { type: "setVolume", volume: 101 }), InvalidActionError);
  assert.throws(() => playerReducer(initialPlayer, { type: "setVolume", volume: 12.5 }), InvalidActionError);
  const bad = { type: "setVolume", volume: -1 } as const;
  assert.throws(
    () => playerReducer(initialPlayer, bad),
    (err) => err instanceof InvalidActionError && err.action === bad,
  );
});

test("setVolume accepts 0 and 100", () => {
  assert.equal(playerReducer(initialPlayer, { type: "setVolume", volume: 0 }).volume, 0);
  assert.equal(playerReducer(initialPlayer, { type: "setVolume", volume: 100 }).volume, 100);
});

test("a reducer that throws leaves the state alone and notifies no one", () => {
  const broken = new Store(initialPlayer, (): PlayerState => {
    throw new Error("boom");
  });
  const before = broken.state;
  let calls = 0;
  broken.subscribe(() => calls++);
  assert.throws(() => broken.dispatch({ type: "stop" }), /boom/);
  assert.equal(broken.state, before);
  assert.equal(calls, 0);
});

test("update merges a patch into a new state object", () => {
  const player = new Store(initialPlayer, playerReducer);
  const before = player.state;
  player.update({ volume: 30, shuffle: true });
  assert.deepEqual(player.state, { track: null, status: "stopped", volume: 30, shuffle: true });
  assert.deepEqual(before, { track: null, status: "stopped", volume: 50, shuffle: false });
  assert.deepEqual(initialPlayer, { track: null, status: "stopped", volume: 50, shuffle: false });
});

test("subscribers hear every change, in order, until they unsubscribe", () => {
  const player = new Store(initialPlayer, playerReducer);
  const heard: string[] = [];
  const unsubscribe = player.subscribe((s) => heard.push(`a:${s.status}`));
  player.subscribe((s) => heard.push(`b:${s.volume}`));
  player.dispatch({ type: "play", track: "Waterloo" });
  unsubscribe();
  player.update({ volume: 80 });
  assert.deepEqual(heard, ["a:playing", "b:50", "b:80"]);
});

test("the store works for any state and action type", () => {
  type CounterAction = { type: "inc" } | { type: "add"; by: number };
  const counter = new Store({ count: 0 }, (s, a: CounterAction) => (a.type === "inc" ? { count: s.count + 1 } : { count: s.count + a.by }));
  counter.dispatch({ type: "inc" });
  counter.dispatch({ type: "add", by: 5 });
  assert.equal(counter.select("count"), 6);
});

function typeChecks(player: Store<PlayerState, PlayerAction>) {
  const volume = player.select("volume");
  const track = player.select("track");
  type _cases = [
    Expect<Equal<typeof volume, number>>,
    Expect<Equal<typeof track, string | null>>,
    Expect<Equal<typeof player.state, Readonly<PlayerState>>>,
    Expect<Equal<Parameters<typeof player.update>[0], Partial<PlayerState>>>,
  ];

  // @ts-expect-error "rewind" is not a PlayerAction
  player.dispatch({ type: "rewind" });
  // @ts-expect-error a play action needs a track
  player.dispatch({ type: "play" });
  // @ts-expect-error "volum" is not a field of PlayerState
  player.update({ volum: 30 });
  // @ts-expect-error volume is a number
  player.update({ volume: "loud" });
  // @ts-expect-error "colour" is not a key of PlayerState
  player.select("colour");
  // @ts-expect-error the state is read-only from outside
  player.state.volume = 0;
  // @ts-expect-error a store's state must be an object
  new Store(42, (s: number) => s);
}
void typeChecks;

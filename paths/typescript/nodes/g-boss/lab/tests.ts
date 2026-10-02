const typedPlayer = new Store(initialPlayer, playerReducer);
const volume = typedPlayer.select("volume");
const track = typedPlayer.select("track");

type cases = [
  Expect<Equal<typeof volume, number>>,
  Expect<Equal<typeof track, string | null>>,
  Expect<Equal<typeof typedPlayer.state, Readonly<PlayerState>>>,
  Expect<Equal<Parameters<typeof typedPlayer.update>[0], Partial<PlayerState>>>,
  Expect<Equal<Parameters<typeof typedPlayer.dispatch>[0], PlayerAction>>,
];

// Never called: these lines are only here for the compiler.
const typeOnlyChecks = () => {
  // @ts-expect-error — "rewind" is not a PlayerAction
  typedPlayer.dispatch({ type: "rewind" });

  // @ts-expect-error — a play action needs a track
  typedPlayer.dispatch({ type: "play" });

  // @ts-expect-error — "volum" is not a field of PlayerState
  typedPlayer.update({ volum: 30 });

  // @ts-expect-error — volume is a number
  typedPlayer.update({ volume: "loud" });

  // @ts-expect-error — "colour" is not a key of PlayerState
  typedPlayer.select("colour");

  // @ts-expect-error — the state is read-only from outside
  typedPlayer.state.volume = 0;
};

test("play, pause and stop move through the states", () => {
  const player = new Store(initialPlayer, playerReducer);
  player.dispatch({ type: "play", track: "Dancing Queen" });
  expect(player.state).toEqual({ track: "Dancing Queen", status: "playing", volume: 50, shuffle: false });
  player.dispatch({ type: "pause" });
  expect(player.select("status")).toBe("paused");
  player.dispatch({ type: "stop" });
  expect(player.state).toEqual({ track: null, status: "stopped", volume: 50, shuffle: false });
});

test("invalid actions throw InvalidActionError", () => {
  const player = new Store(initialPlayer, playerReducer);
  expect(() => player.dispatch({ type: "pause" })).toThrow("pause: Nothing is playing");
  expect(() => player.dispatch({ type: "setVolume", volume: 101 })).toThrow("setVolume: Volume must be 0-100");
  expect(() => player.dispatch({ type: "setVolume", volume: 12.5 })).toThrow("setVolume: Volume must be 0-100");
  const bad = { type: "setVolume", volume: -1 } as const;
  try {
    playerReducer(initialPlayer, bad);
  } catch (err) {
    expect(err).toBeInstanceOf(InvalidActionError);
    if (err instanceof InvalidActionError) {
      expect(err.name).toBe("InvalidActionError");
      expect(err.action).toBe(bad);
    }
  }
});

test("setVolume accepts 0 and 100", () => {
  expect(playerReducer(initialPlayer, { type: "setVolume", volume: 0 }).volume).toBe(0);
  expect(playerReducer(initialPlayer, { type: "setVolume", volume: 100 }).volume).toBe(100);
});

test("a throwing reducer leaves the state alone and notifies no one", () => {
  const player = new Store(initialPlayer, playerReducer);
  const before = player.state;
  let calls = 0;
  player.subscribe(() => calls++);
  expect(() => player.dispatch({ type: "pause" })).toThrow();
  expect(player.state).toBe(before);
  expect(calls).toBe(0);
});

test("update merges a patch into a new state object", () => {
  const player = new Store(initialPlayer, playerReducer);
  const before = player.state;
  player.update({ volume: 30, shuffle: true });
  expect(player.state).toEqual({ track: null, status: "stopped", volume: 30, shuffle: true });
  expect(before).toEqual({ track: null, status: "stopped", volume: 50, shuffle: false });
  expect(initialPlayer).toEqual({ track: null, status: "stopped", volume: 50, shuffle: false });
});

test("subscribers hear every change, in order, until they unsubscribe", () => {
  const player = new Store(initialPlayer, playerReducer);
  const heard: string[] = [];
  const unsubscribe = player.subscribe((s) => heard.push(`a:${s.status}`));
  player.subscribe((s) => heard.push(`b:${s.volume}`));
  player.dispatch({ type: "play", track: "Waterloo" });
  unsubscribe();
  player.update({ volume: 80 });
  expect(heard).toEqual(["a:playing", "b:50", "b:80"]);
});

test("the store works for any state and action type", () => {
  type CounterAction = { type: "inc" } | { type: "add"; by: number };
  const counter = new Store({ count: 0 }, (s, a: CounterAction) =>
    a.type === "inc" ? { count: s.count + 1 } : { count: s.count + a.by },
  );
  counter.dispatch({ type: "inc" });
  counter.dispatch({ type: "add", by: 5 });
  expect(counter.select("count")).toBe(6);
});

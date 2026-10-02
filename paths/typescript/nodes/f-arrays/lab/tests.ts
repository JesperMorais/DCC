const queue = Object.freeze(["Intro", "Blue", "Echo", "Outro"]);

type cases = [
  Expect<Equal<ReturnType<typeof playNext>, string[]>>,
  Expect<Equal<Parameters<typeof moveDown>, [queue: readonly string[], index: number]>>,
];

test("upNext returns the first songs", () => {
  expect(upNext(queue, 2)).toEqual(["Intro", "Blue"]);
  expect(upNext(queue, 10)).toEqual(["Intro", "Blue", "Echo", "Outro"]);
  expect(upNext(queue, 0)).toEqual([]);
});

test("playNext puts a new song at the front", () => {
  expect(playNext(queue, "Neon")).toEqual(["Neon", "Intro", "Blue", "Echo", "Outro"]);
});

test("playNext moves a queued song instead of duplicating it", () => {
  expect(playNext(queue, "Echo")).toEqual(["Echo", "Intro", "Blue", "Outro"]);
  expect(playNext(queue, "Intro")).toEqual(["Intro", "Blue", "Echo", "Outro"]);
});

test("moveDown swaps a song with the next one", () => {
  expect(moveDown(queue, 1)).toEqual(["Intro", "Echo", "Blue", "Outro"]);
  expect(moveDown(queue, 0)).toEqual(["Blue", "Intro", "Echo", "Outro"]);
});

test("moveDown leaves the last or an invalid position alone", () => {
  expect(moveDown(queue, 3)).toEqual(["Intro", "Blue", "Echo", "Outro"]);
  expect(moveDown(queue, 7)).toEqual(["Intro", "Blue", "Echo", "Outro"]);
  expect(moveDown(queue, -1)).toEqual(["Intro", "Blue", "Echo", "Outro"]);
});

test("every helper returns a new array, even when nothing changes", () => {
  const mine = ["A", "B"];
  expect(upNext(mine, 10)).not.toBe(mine);
  expect(playNext(mine, "A")).not.toBe(mine);
  expect(moveDown(mine, 1)).not.toBe(mine);
});

test("the original queue is never changed", () => {
  const mine = ["A", "B", "C"];
  upNext(mine, 1);
  playNext(mine, "C");
  playNext(mine, "D");
  moveDown(mine, 0);
  expect(mine).toEqual(["A", "B", "C"]);
});

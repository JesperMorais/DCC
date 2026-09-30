const sleep = (ms: number) => new Promise<void>((r) => setTimeout(r, ms));

function tracker() {
  const t = { running: 0, maxRunning: 0, started: 0 };
  const run = async <R>(ms: number, value: R): Promise<R> => {
    t.started++;
    t.running++;
    t.maxRunning = Math.max(t.maxRunning, t.running);
    await sleep(ms);
    t.running--;
    return value;
  };
  return { t, run };
}

test("maps every item, keeping input order", async () => {
  const out = await mapLimit([1, 2, 3, 4], 2, async (n) => n * 10);
  expect(out).toEqual([10, 20, 30, 40]);
});

test("never exceeds the limit", async () => {
  const { t, run } = tracker();
  await mapLimit([1, 2, 3, 4, 5, 6, 7], 3, (n) => run(5, n));
  expect(t.maxRunning).toBe(3);
  expect(t.started).toBe(7);
});

test("order is by input, not by finish time", async () => {
  const delays = [30, 5, 15, 1];
  const out = await mapLimit(delays, 2, async (ms) => {
    await sleep(ms);
    return `done ${ms}`;
  });
  expect(out).toEqual(["done 30", "done 5", "done 15", "done 1"]);
});

test("starts the next item as soon as a slot frees up", async () => {
  // limit 2: [40, 5, 5, 5] should take ~40ms (items 2–4 flow through the 2nd slot),
  // not ~45ms+ like fixed batches of 2 would.
  const { t, run } = tracker();
  const startedAtCheck: number[] = [];
  const p = mapLimit([40, 5, 5, 5], 2, (ms) => run(ms, ms));
  await sleep(25);
  startedAtCheck.push(t.started);
  await p;
  expect(startedAtCheck[0]).toBe(4);
});

test("empty input resolves to []", async () => {
  expect(await mapLimit([], 4, async (x: number) => x)).toEqual([]);
});

test("limit larger than the list is fine", async () => {
  const { t, run } = tracker();
  const out = await mapLimit(["a", "b"], 10, (s) => run(2, s.toUpperCase()));
  expect(out).toEqual(["A", "B"]);
  expect(t.maxRunning).toBe(2);
});

test("rejects if any call rejects", async () => {
  let message = "no error";
  try {
    await mapLimit([1, 2, 3], 2, async (n) => {
      await sleep(1);
      if (n === 2) throw new Error("item 2 failed");
      return n;
    });
  } catch (e) {
    message = (e as Error).message;
  }
  expect(message).toBe("item 2 failed");
});

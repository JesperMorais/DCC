function flaky<T>(failures: number, value: T) {
  let calls = 0;
  const fn = async (): Promise<T> => {
    calls++;
    await new Promise((r) => setTimeout(r, 1));
    if (calls <= failures) throw new Error(`fail #${calls}`);
    return value;
  };
  return { fn, calls: () => calls };
}

async function errorOf(p: Promise<unknown>): Promise<string> {
  try {
    await p;
    return "no error";
  } catch (e) {
    return e instanceof Error ? e.message : String(e);
  }
}

test("succeeds on the first try and calls fn once", async () => {
  const f = flaky(0, 42);
  expect(await retry(f.fn, 3)).toBe(42);
  expect(f.calls()).toBe(1);
});

test("recovers after two failures", async () => {
  const f = flaky(2, "ok");
  expect(await retry(f.fn, 3)).toBe("ok");
  expect(f.calls()).toBe(3);
});

test("rejects with the LAST error when every attempt fails", async () => {
  const f = flaky(10, "never");
  expect(await errorOf(retry(f.fn, 2))).toBe("fail #2");
  expect(f.calls()).toBe(2);
});

test("attempts = 1 means no retry", async () => {
  const f = flaky(1, "x");
  expect(await errorOf(retry(f.fn, 1))).toBe("fail #1");
  expect(f.calls()).toBe(1);
});

test("stops calling fn as soon as it succeeds", async () => {
  const f = flaky(1, true);
  expect(await retry(f.fn, 5)).toBe(true);
  expect(f.calls()).toBe(2);
});

// The generic flows through: retry keeps the result type.
const typed = retry(async () => ({ id: 1 }), 2);
type retry_cases = [Expect<Equal<typeof typed, Promise<{ id: number }>>>];

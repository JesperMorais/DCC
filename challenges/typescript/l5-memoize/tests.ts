function counted<A, R>(fn: (arg: A) => R) {
  const wrapper = (arg: A) => {
    wrapper.calls++;
    return fn(arg);
  };
  wrapper.calls = 0;
  return wrapper;
}

test("returns the same results as the original", () => {
  const square = memoize((n: number) => n * n);
  expect(square(3)).toBe(9);
  expect(square(-4)).toBe(16);
});

test("calls fn only once per argument", () => {
  const spy = counted((n: number) => n * n);
  const fast = memoize(spy);
  fast(4);
  fast(4);
  fast(4);
  expect(spy.calls).toBe(1);
  fast(5);
  expect(spy.calls).toBe(2);
});

test("caches falsy results too", () => {
  const spy = counted((s: string) => (s === "none" ? undefined : s.length));
  const fast = memoize(spy);
  expect(fast("none")).toBeUndefined();
  expect(fast("none")).toBeUndefined();
  expect(fast("")).toBe(0);
  expect(fast("")).toBe(0);
  expect(spy.calls).toBe(2);
});

test("each memoized function has its own cache", () => {
  const spy = counted((n: number) => n + 1);
  const a = memoize(spy);
  const b = memoize(spy);
  a(1);
  b(1);
  expect(spy.calls).toBe(2);
});

test("object arguments are compared by reference", () => {
  const spy = counted((o: { n: number }) => o.n * 10);
  const fast = memoize(spy);
  const key = { n: 1 };
  fast(key);
  fast(key);
  fast({ n: 1 }); // a different object
  expect(spy.calls).toBe(2);
});

// Types are preserved by the wrapper.
const memoLen = memoize((s: string) => s.length);
type memoize_cases = [Expect<Equal<typeof memoLen, (arg: string) => number>>];

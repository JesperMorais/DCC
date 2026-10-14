const rates: Record<string, number> = { EUR: 1, SEK: 11.5, USD: 1.1 };

function countCalls<A extends unknown[], R>(fn: (...args: A) => R) {
  const spy = (...args: A) => {
    spy.calls++;
    return fn(...args);
  };
  spy.calls = 0;
  return spy;
}

test("same arguments run fn only once", () => {
  const spy = countCalls((amount: number, currency: string) => amount * rates[currency]);
  const convert = memoizeArgs(spy);
  expect(convert(100, "EUR")).toBe(100);
  expect(convert(100, "EUR")).toBe(100);
  expect(spy.calls).toBe(1);
});

test("a different first argument runs fn again", () => {
  const spy = countCalls((amount: number, currency: string) => amount * rates[currency]);
  const convert = memoizeArgs(spy);
  expect(convert(100, "EUR")).toBe(100);
  expect(convert(200, "EUR")).toBe(200);
  expect(spy.calls).toBe(2);
});

test("a different second argument runs fn again", () => {
  const convert = memoizeArgs((amount: number, currency: string) => amount * rates[currency]);
  expect(convert(100, "EUR")).toBe(100);
  expect(convert(100, "SEK")).toBe(1150);
  expect(convert(100, "USD")).toBeCloseTo(110);
});

test("objects are compared by value", () => {
  const spy = countCalls((size: { w: number; h: number }) => size.w * size.h);
  const area = memoizeArgs(spy);
  expect(area({ w: 2, h: 3 })).toBe(6);
  expect(area({ w: 2, h: 3 })).toBe(6);
  expect(area({ w: 4, h: 5 })).toBe(20);
  expect(spy.calls).toBe(2);
});

test("falsy results are cached", () => {
  const spy = countCalls((s: string) => s.length);
  const len = memoizeArgs(spy);
  expect(len("")).toBe(0);
  expect(len("")).toBe(0);
  expect(spy.calls).toBe(1);
});

test("each memoized function has its own cache", () => {
  const spy = countCalls((n: number) => n * 2);
  const a = memoizeArgs(spy);
  const b = memoizeArgs(spy);
  a(1);
  b(1);
  expect(spy.calls).toBe(2);
});

const memoConvert = memoizeArgs((amount: number, currency: string) => `${amount} ${currency}`);
type memo_args_cases = [Expect<Equal<Parameters<typeof memoConvert>, [amount: number, currency: string]>>];

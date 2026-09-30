test("ok and err build the two shapes", () => {
  expect(ok(5)).toEqual({ ok: true, value: 5 });
  expect(err("boom")).toEqual({ ok: false, error: "boom" });
});

test("parses valid ports, ignoring whitespace", () => {
  expect(parsePort("8080")).toEqual({ ok: true, value: 8080 });
  expect(parsePort(" 443 ")).toEqual({ ok: true, value: 443 });
  expect(parsePort("65535")).toEqual({ ok: true, value: 65535 });
});

test("rejects things that aren't whole numbers", () => {
  expect(parsePort("abc")).toEqual({ ok: false, error: "not a number" });
  expect(parsePort("12.5")).toEqual({ ok: false, error: "not a number" });
  expect(parsePort("-80")).toEqual({ ok: false, error: "not a number" });
  expect(parsePort("")).toEqual({ ok: false, error: "not a number" });
});

test("rejects out-of-range numbers", () => {
  expect(parsePort("0")).toEqual({ ok: false, error: "out of range" });
  expect(parsePort("70000")).toEqual({ ok: false, error: "out of range" });
});

test("unwrapOr picks value or fallback", () => {
  expect(unwrapOr(parsePort("22"), 3000)).toBe(22);
  expect(unwrapOr(parsePort("nope"), 3000)).toBe(3000);
});

test("narrowing on .ok gives access to value / error", () => {
  const r = parsePort("99999");
  const message = r.ok ? `port ${r.value}` : `bad: ${r.error}`;
  expect(message).toBe("bad: out of range");
});

type result_cases = [
  Expect<Equal<Result<number, string>, { ok: true; value: number } | { ok: false; error: string }>>,
];

function _resultTypeChecks(r: Result<number, string>) {
  // @ts-expect-error — can't read .value before checking .ok
  r.value;
}

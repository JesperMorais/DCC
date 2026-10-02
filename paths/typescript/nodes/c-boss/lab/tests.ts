const incident = [
  "09:15:03 GET /api/orders 200 120ms",
  "09:15:04 POST /api/checkout 503 2400ms",
  "09:15:04 GET /health 200 1ms",
  "",
  "09:15:05 GET /api/orders 304 15ms",
  "09:15:06 POST /api/checkout 503 2400ms",
  "panic: runtime error",
  "   ",
  "  09:15:07 DELETE /api/cart/7 404 33ms  ",
  "09:15:08 PATCH /api/cart/7 200 10ms",
  "09:15:09 GET /health 200 2ms",
  "",
].join("\n");

type cases = [
  Expect<Equal<Method, "GET" | "POST" | "PUT" | "DELETE">>,
  Expect<Equal<ReturnType<typeof parseRequest>, LogRequest | null>>,
];

// @ts-expect-error — "GTE" is not a Method
const typo: LogRequest = { time: "09:00:00", method: "GTE", path: "/", status: 200, ms: 1 };

test("statusClass buckets the status", () => {
  expect(statusClass(200)).toBe("2xx");
  expect(statusClass(299)).toBe("2xx");
  expect(statusClass(301)).toBe("3xx");
  expect(statusClass(404)).toBe("4xx");
  expect(statusClass(503)).toBe("5xx");
});

test("parseRequest reads every field, numbers as numbers", () => {
  expect(parseRequest("  09:15:03 GET /api/orders 200 120ms ")).toEqual({
    time: "09:15:03",
    method: "GET",
    path: "/api/orders",
    status: 200,
    ms: 120,
  });
});

test("parseRequest rejects lines that don't match exactly", () => {
  expect(parseRequest("09:15:08 PATCH /api/cart/7 200 10ms")).toBeNull();
  expect(parseRequest("09:15 GET /a 200 10ms")).toBeNull();
  expect(parseRequest("09:15:08 GET a 200 10ms")).toBeNull();
  expect(parseRequest("09:15:08 GET /a 600 10ms")).toBeNull();
  expect(parseRequest("09:15:08 GET /a 200 10")).toBeNull();
  expect(parseRequest("09:15:08 GET /a 200 1.5ms")).toBeNull();
  expect(parseRequest("09:15:08  GET /a 200 10ms")).toBeNull();
});

test("analyse counts requests, status classes and distinct paths", () => {
  const report = analyse(incident);
  expect(report.requests).toBe(7);
  expect(report.byStatus).toEqual({ "2xx": 3, "3xx": 1, "4xx": 1, "5xx": 2 });
  expect(report.distinctPaths).toBe(4);
});

test("analyse lists unreadable lines by 1-based number, skipping blanks", () => {
  expect(analyse(incident).unreadable).toEqual([7, 10]);
});

test("analyse finds the slowest request (first on a tie) and the average", () => {
  const report = analyse(incident);
  expect(report.slowest).toEqual(["/api/checkout", 2400]);
  expect(report.averageMs).toBe("710.1");
});

test("ignorePaths leaves those requests out of every field", () => {
  const report = analyse(incident, { ignorePaths: ["/health"] });
  expect(report.requests).toBe(5);
  expect(report.byStatus).toEqual({ "2xx": 1, "3xx": 1, "4xx": 1, "5xx": 2 });
  expect(report.distinctPaths).toBe(3);
  expect(report.unreadable).toEqual([7, 10]);
  expect(report.averageMs).toBe("993.6");
});

test("an empty or unreadable log", () => {
  expect(analyse("")).toEqual({
    requests: 0,
    unreadable: [],
    byStatus: { "2xx": 0, "3xx": 0, "4xx": 0, "5xx": 0 },
    distinctPaths: 0,
    slowest: null,
    averageMs: "n/a",
  });
  const junk = analyse("hello\nworld");
  expect(junk.unreadable).toEqual([1, 2]);
  expect(junk.slowest).toBeNull();
});

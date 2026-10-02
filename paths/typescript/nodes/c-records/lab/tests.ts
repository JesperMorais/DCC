const log: readonly string[] = [
  "09:15 ada /pricing",
  "09:16 linus /docs",
  "  09:20 ada /pricing  ",
  "GET /wp-admin",
  "09:21 grace /docs",
  "09:22 linus /docs",
  "",
  "09:30 Ada /pricing",
  "09:31 grace /",
];

type cases = [
  Expect<Equal<ReturnType<typeof viewsPerPage>, Record<string, number>>>,
  Expect<Equal<ReturnType<typeof uniqueVisitors>, Map<string, number>>>,
];

test("parseView reads time, user and path", () => {
  expect(parseView("09:15 ada /pricing")).toEqual({ time: "09:15", user: "ada", path: "/pricing" });
  expect(parseView("  23:59 grace /  ")).toEqual({ time: "23:59", user: "grace", path: "/" });
});

test("parseView rejects junk lines", () => {
  expect(parseView("GET /wp-admin")).toBeNull();
  expect(parseView("9:15 ada /pricing")).toBeNull();
  expect(parseView("09:15 Ada /pricing")).toBeNull();
  expect(parseView("09:15 ada pricing")).toBeNull();
  expect(parseView("09:15 ada /pricing extra")).toBeNull();
  expect(parseView("")).toBeNull();
});

test("viewsPerPage counts every view per path", () => {
  expect(viewsPerPage(log)).toEqual({ "/pricing": 2, "/docs": 3, "/": 1 });
});

test("viewsPerPage of no lines is an empty object", () => {
  expect(viewsPerPage([])).toEqual({});
});

test("uniqueVisitors counts different users per path", () => {
  const unique = uniqueVisitors(log);
  expect(unique.get("/pricing")).toBe(1);
  expect(unique.get("/docs")).toBe(2);
  expect(unique.get("/")).toBe(1);
  expect(unique.size).toBe(3);
});

test("uniqueVisitors keeps first-seen order", () => {
  expect([...uniqueVisitors(log).keys()]).toEqual(["/pricing", "/docs", "/"]);
});

test("the input is not modified", () => {
  const lines = ["09:15 ada /a", "09:16 ada /b"];
  viewsPerPage(lines);
  uniqueVisitors(lines);
  expect(lines).toEqual(["09:15 ada /a", "09:16 ada /b"]);
});

test("accepts a valid user (with and without email)", () => {
  expect(isUser({ id: 1, name: "Ada", roles: ["admin"] })).toBe(true);
  expect(isUser({ id: 2, name: "Linus", email: "l@k.org", roles: [] })).toBe(true);
});

test("allows extra properties", () => {
  expect(isUser({ id: 1, name: "Ada", roles: [], age: 36 })).toBe(true);
});

test("rejects non-objects and null", () => {
  expect(isUser(null)).toBe(false);
  expect(isUser("Ada")).toBe(false);
  expect(isUser(undefined)).toBe(false);
  expect(isUser([])).toBe(false);
});

test("rejects wrong field types", () => {
  expect(isUser({ id: "1", name: "Ada", roles: [] })).toBe(false);
  expect(isUser({ id: 1, roles: [] })).toBe(false);
  expect(isUser({ id: 1, name: "Ada", email: 42, roles: [] })).toBe(false);
  expect(isUser({ id: 1, name: "Ada" })).toBe(false);
});

test("rejects roles containing a non-string", () => {
  expect(isUser({ id: 1, name: "Ada", roles: ["admin", 2] })).toBe(false);
});

test("assertUser narrows parsed JSON", () => {
  const data: unknown = JSON.parse('{"id":7,"name":"Grace","roles":["navy"]}');
  assertUser(data);
  // This line only compiles because assertUser narrowed `data` to User:
  expect(data.name.toUpperCase()).toBe("GRACE");
});

test("assertUser throws 'Invalid user' for bad input", () => {
  expect(() => assertUser({ id: 1 })).toThrow("Invalid user");
  expect(() => assertUser(null)).toThrow("Invalid user");
});

const cfg = {
  db: { host: "localhost", port: 5432, replicas: 0, password: null },
  users: [{ name: "Ada", tags: ["math"] }, { name: "Linus" }],
  debug: false,
};

test("reads nested object properties", () => {
  expect(getPath(cfg, "db.host")).toBe("localhost");
  expect(getPath(cfg, "db.port")).toBe(5432);
});

test("uses numeric segments for arrays", () => {
  expect(getPath(cfg, "users.1.name")).toBe("Linus");
  expect(getPath(cfg, "users.0.tags.0")).toBe("math");
});

test("returns falsy values as-is", () => {
  expect(getPath(cfg, "db.replicas")).toBe(0);
  expect(getPath(cfg, "db.password")).toBeNull();
  expect(getPath(cfg, "debug")).toBe(false);
});

test("empty path returns the object itself", () => {
  expect(getPath(cfg, "")).toBe(cfg);
});

test("missing steps give undefined instead of throwing", () => {
  expect(getPath(cfg, "db.user.name")).toBeUndefined();
  expect(getPath(cfg, "users.5.name")).toBeUndefined();
  expect(getPath(cfg, "db.password.length")).toBeUndefined();
  expect(getPath(null, "a")).toBeUndefined();
});

test("does not descend into primitives", () => {
  expect(getPath(cfg, "db.host.length")).toBeUndefined();
});

const ada: User = { id: 1, name: "Ada", email: "ada@example.com", role: "member", updatedAt: 0 };

type cases = [
  Expect<Equal<UserPatch, { name?: string; email?: string; role?: "admin" | "member" }>>,
];

// @ts-expect-error — the id can't be patched
updateUser(ada, { id: 2 }, 1);

// @ts-expect-error — updatedAt is set by the server, not the client
updateUser(ada, { updatedAt: 5 }, 1);

test("replaces the patched field and stamps updatedAt", () => {
  expect(updateUser(ada, { role: "admin" }, 100)).toEqual({
    id: 1,
    name: "Ada",
    email: "ada@example.com",
    role: "admin",
    updatedAt: 100,
  });
});

test("an empty patch only changes updatedAt", () => {
  expect(updateUser(ada, {}, 50)).toEqual({ ...ada, updatedAt: 50 });
});

test("normalises the email", () => {
  expect(updateUser(ada, { email: "  Ada@Lovelace.dev " }, 200).email).toBe("ada@lovelace.dev");
});

test("patches several fields at once", () => {
  const next = updateUser(ada, { name: "Ada L.", role: "admin" }, 300);
  expect(next.name).toBe("Ada L.");
  expect(next.role).toBe("admin");
  expect(next.email).toBe("ada@example.com");
});

test("does not mutate the original", () => {
  const next = updateUser(ada, { name: "Changed", email: "X@Y.Z" }, 400);
  expect(next === ada).toBe(false);
  expect(ada).toEqual({ id: 1, name: "Ada", email: "ada@example.com", role: "member", updatedAt: 0 });
});

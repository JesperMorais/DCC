function fakeClock() {
  let t = 1000;
  return { now: () => t, advance: (ms: number) => (t += ms) };
}

async function errorOf(p: Promise<unknown>): Promise<string> {
  try {
    await p;
    return "no error";
  } catch (e) {
    return e instanceof Error ? e.message : String(e);
  }
}

test("a fresh value comes from the cache", async () => {
  const clock = fakeClock();
  const users = createCachedFetcher(async (id: string) => `user ${id}`, 5000, clock.now);
  expect(await users.get("7")).toBe("user 7");
  clock.advance(4999);
  expect(await users.get("7")).toBe("user 7");
  expect(users.stats).toEqual({ hits: 1, fetches: 1 });
});

test("concurrent requests for one key share a single fetch", async () => {
  const users = createCachedFetcher(async (id: string) => `user ${id}`, 5000);
  const [a, b, c] = await Promise.all([users.get("7"), users.get("7"), users.get("8")]);
  expect([a, b, c]).toEqual(["user 7", "user 7", "user 8"]);
  expect(users.stats.fetches).toBe(2);
});

test("an expired value is fetched again", async () => {
  const clock = fakeClock();
  let version = 0;
  const users = createCachedFetcher(async (id: string) => `user ${id} v${++version}`, 5000, clock.now);
  expect(await users.get("7")).toBe("user 7 v1");
  clock.advance(5000);
  expect(await users.get("7")).toBe("user 7 v2");
});

test("invalidate forces a refetch", async () => {
  let version = 0;
  const users = createCachedFetcher(async (id: string) => `user ${id} v${++version}`, 5000);
  await users.get("7");
  users.invalidate("7");
  expect(await users.get("7")).toBe("user 7 v2");
});

test("a failed fetch rejects every waiting caller", async () => {
  const users = createCachedFetcher(async (id: string): Promise<string> => {
    throw new Error(`503 for ${id}`);
  }, 5000);
  const errors = await Promise.all([errorOf(users.get("7")), errorOf(users.get("7"))]);
  expect(errors).toEqual(["503 for 7", "503 for 7"]);
  expect(users.stats.fetches).toBe(1);
});

test("after a failure, the next request tries again", async () => {
  let attempts = 0;
  const users = createCachedFetcher(async (id: string) => {
    attempts++;
    if (attempts === 1) throw new Error("503");
    return `user ${id}`;
  }, 5000);
  expect(await errorOf(users.get("7"))).toBe("503");
  expect(await users.get("7")).toBe("user 7");
  expect(attempts).toBe(2);
});

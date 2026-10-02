interface Conn {
  id: number;
  host: string;
}

const conns: Conn[] = [
  { id: 1, host: "db-1" },
  { id: 2, host: "db-2" },
];

const typed = new Pool(conns);
const host = typed.use((c) => c.host);

type cases = [
  Expect<Equal<ReturnType<typeof typed.acquire>, Conn>>,
  Expect<Equal<typeof host, string>>,
];

// Never called: these lines are only here for the compiler.
const typeOnlyChecks = () => {
  // @ts-expect-error — a Pool<Conn> only takes Conns back
  typed.release("db-1");

  // @ts-expect-error — available is a getter with no setter
  typed.available = 10;
};

test("acquire hands out resources in order", () => {
  const pool = new Pool(["conn-a", "conn-b", "conn-c"]);
  expect(pool.available).toBe(3);
  expect(pool.acquire()).toBe("conn-a");
  expect(pool.acquire()).toBe("conn-b");
  expect(pool.available).toBe(1);
});

test("a released resource goes to the back of the line", () => {
  const pool = new Pool(["conn-a", "conn-b", "conn-c"]);
  const a = pool.acquire();
  pool.release(a);
  expect(pool.available).toBe(3);
  expect(pool.acquire()).toBe("conn-b");
  expect(pool.acquire()).toBe("conn-c");
  expect(pool.acquire()).toBe("conn-a");
});

test("an empty pool throws PoolExhaustedError", () => {
  const pool = new Pool(conns);
  pool.acquire();
  pool.acquire();
  expect(() => pool.acquire()).toThrow("All 2 resources are in use");
  try {
    pool.acquire();
  } catch (err) {
    expect(err).toBeInstanceOf(PoolExhaustedError);
    expect(err).toBeInstanceOf(Error);
    if (err instanceof PoolExhaustedError) {
      expect(err.name).toBe("PoolExhaustedError");
      expect(err.capacity).toBe(2);
    }
  }
});

test("releasing something not checked out throws", () => {
  const pool = new Pool(["conn-a", "conn-b"]);
  expect(() => pool.release("conn-a")).toThrow("Resource is not checked out");
  const a = pool.acquire();
  pool.release(a);
  expect(() => pool.release(a)).toThrow("Resource is not checked out");
  expect(pool.available).toBe(2);
});

test("use returns what fn returns and gives the resource back", () => {
  const pool = new Pool(conns);
  expect(pool.use((c) => c.id * 10)).toBe(10);
  expect(pool.available).toBe(2);
});

test("use releases the resource even when fn throws", () => {
  const pool = new Pool(conns);
  expect(() =>
    pool.use(() => {
      throw new Error("query failed");
    }),
  ).toThrow("query failed");
  expect(pool.available).toBe(2);
});

test("state is private and the input is not modified", () => {
  const list = ["conn-a", "conn-b"];
  const pool = new Pool(list);
  pool.acquire();
  expect(list).toEqual(["conn-a", "conn-b"]);
  expect(Object.keys(pool)).toEqual([]);
});

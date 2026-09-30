const typed = new Stack<number>();

type cases = [
  Expect<Equal<ReturnType<typeof typed.pop>, number>>,
  Expect<Equal<ReturnType<typeof typed.peek>, number | undefined>>,
];

// @ts-expect-error — a Stack<number> only accepts numbers
typed.push("nope");

// @ts-expect-error — the storage must be private
typed.items;

test("pops in last-in, first-out order", () => {
  const history = new Stack<string>();
  history.push("type 'Hello'");
  history.push("bold 'Hello'");
  expect(history.pop()).toBe("bold 'Hello'");
  expect(history.pop()).toBe("type 'Hello'");
});

test("size tracks pushes and pops", () => {
  const s = new Stack<number>();
  expect(s.size).toBe(0);
  s.push(1);
  s.push(2);
  s.push(3);
  expect(s.size).toBe(3);
  s.pop();
  expect(s.size).toBe(2);
});

test("peek looks without removing", () => {
  const s = new Stack<string>();
  s.push("a");
  s.push("b");
  expect(s.peek()).toBe("b");
  expect(s.size).toBe(2);
});

test("peek on an empty stack is undefined", () => {
  expect(new Stack<string>().peek()).toBeUndefined();
});

test("pop on an empty stack throws", () => {
  const s = new Stack<number>();
  expect(() => s.pop()).toThrow("Stack is empty");
  s.push(7);
  s.pop();
  expect(() => s.pop()).toThrow("Stack is empty");
});

test("separate stacks don't share items", () => {
  const a = new Stack<number>();
  const b = new Stack<number>();
  a.push(1);
  expect(b.size).toBe(0);
});

Every test you've passed so far was written by someone else. In the project after this workshop there are none: you write them. That sounds like extra work, but it's the part that lets you change code without fear. This workshop is the how, with commands to type and the output you'll see.

### From a user story to test cases

A milestone reads like a story: *"I can subscribe for one emission only."* Turn each sentence into cases, in three kinds:

- **Happy path:** subscribe once, emit, the listener ran with the payload.
- **Edge:** emit twice. It ran *once*. (This is the one the sentence is really about.)
- **Error or refusal:** what should fail? Here, nothing at runtime, but `bus.once("nope", …)` shouldn't compile.

Write the cases as a list of names before any code. They're your to-do list.

### One behaviour per test, named as a sentence

Each test has three parts, **arrange, act, assert**:

```ts
test("once fires a single time", () => {
  const bus = new EventBus<ChatEvents>();       // arrange
  const seen: string[] = [];
  bus.once("joined", (name) => seen.push(name));

  bus.emit("joined", "ada");                    // act
  bus.emit("joined", "grace");

  assert.deepEqual(seen, ["ada"]);              // assert
});
```

The name is a sentence about behaviour, so a failure reads like a bug report: `✖ once fires a single time`. If a test needs "and" in its name, it's probably two tests.

**Testing callbacks:** don't try to inspect a listener. Make it **record** what it got into an array, then compare the array. That one trick also covers order (`[1, 2]`) and **"not called"**: `assert.deepEqual(seen, [])`, or a counter that must stay `0`.

### Worked example: porting the boss lab's tests

The Generics boss lab used `expect(...)`. Your project uses `node:assert/strict`. The translation is mechanical:

| Boss lab | node:assert |
|---|---|
| `expect(x).toBe(y)` | `assert.equal(x, y)` (same value, `===`) |
| `expect(x).toEqual(y)` | `assert.deepEqual(x, y)` (same structure) |
| `expect(fn).toThrow("msg")` | `assert.throws(fn, { message: "msg" })` |
| `expect(err).toBeInstanceOf(E)` | `assert.throws(fn, E)` |

So this boss test:

```ts
expect(() => player.dispatch({ type: "pause" })).toThrow("pause: Nothing is playing");
expect(player.select("status")).toBe("stopped");
```

becomes:

```ts
assert.throws(() => player.dispatch({ type: "pause" }), { name: "InvalidActionError", message: "pause: Nothing is playing" });
assert.equal(player.select("status"), "stopped");
```

Note the `() =>`: `assert.throws` needs a function it can call and catch. Pass `player.dispatch(...)` directly and the error escapes before `assert` ever sees it.

Then **see it fail**. A test you've only ever seen pass might not test anything. Break the code on purpose (comment out the `throw`) and run `npm run test:m1`:

```
✖ pause when stopped is rejected
  AssertionError [ERR_ASSERTION]: Missing expected exception (InvalidActionError).
      at TestContext.<anonymous> (…/tests/m1-store.test.ts:6:10)
```

Good: the test notices. Put the `throw` back, see green, and port the next one. One test, run, next.

While you work on one test, run only that one: `npm run test:one -- "pause when stopped" tests/m1-store.test.ts` (part of the test's name, then its file).

### Type-level tests, and the trap

Some promises are about types: "`select("colour")` doesn't compile." You test that with a line that **must** fail:

```ts
function typeChecks(player: Store<PlayerState, PlayerAction>) {
  player.select("volume");          // positive: this one must compile
  // @ts-expect-error "colour" is not a key of PlayerState
  player.select("colour");
}
void typeChecks;
```

The function is never called, so nothing runs; `void typeChecks;` just keeps your editor from calling it unused. If `select` gets loosened to any `string`, the line compiles and `npm run check` says:

```
tests/m1-store.test.ts(4,1): error TS2578: Unused '@ts-expect-error' directive.
```

**The trap:** `@ts-expect-error` accepts *any* error. Misspell the method, `player.selct("colour")`, and the line still fails to compile, the directive is happy, and your test checks nothing. So:

1. Write the line **without** the directive.
2. Run `npm run check` and read the error:
   `error TS2551: Property 'selct' does not exist on type 'Store<…>'. Did you mean 'select'?`
3. Is that the error you meant? Here, no. Fix the typo, read again:
   `error TS2345: Argument of type '"colour"' is not assignable to parameter of type 'keyof PlayerState'.`
4. Now add the directive.

And keep the positive line next to it. A store whose `select` accepts *nothing* would pass every negative test.

For **exact** types, the starter's `tests/type-helpers.ts` has two lines:

```ts
export type Equal<X, Y> = (<T>() => T extends X ? 1 : 2) extends <T>() => T extends Y ? 1 : 2 ? true : false;
export type Expect<T extends true> = T;
```

You don't need to follow how `Equal` works; it's a well-known trick. Use it like this:

```ts
const volume = player.select("volume");
type _cases = [Expect<Equal<typeof volume, number>>];
```

That's stricter than `const v: number = player.select("volume")`, which `any` would also pass.

### Design the API by using it first

Before writing a class, write the test that uses it, the way you *wish* you could call it:

```ts
const off = store.watch("count", (next, prev) => seen.push([prev, next]));
```

Reading that line tells you the method's name, that it takes a key of the state, what the listener gets, and that it returns an unsubscribe. If the call looks awkward in the test, it'll be awkward everywhere. Change it now, while it costs nothing.

### Gotchas

- `assert.equal` on two objects compares identity, so two equal-looking objects fail. Use `deepEqual`, unless identity is the point ("the state is the very same object").
- A test that passes before you've written the code is suspicious. Make it fail once.
- Slow is fine. Writing the case list, then one test at a time, is how this is done well.

### In the wild

- **DefinitelyTyped**, home of every `@types/…` package you install, tests its types with `@ts-expect-error` lines and `$ExpectType` comments: no runtime at all.
- **Node.js** ships `node:test` and `node:assert`; many libraries need nothing else.
- **Redux Toolkit, Zod and tRPC** keep type-level test files (with `Equal`-style helpers or `expectTypeOf`) whose whole job is to fail compilation when someone loosens a type.
- **Test-first API design** is how many teams write a new module: the usage in a test file is the first draft of the docs.

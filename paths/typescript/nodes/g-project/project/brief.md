In the boss you built a store for one music player. This time you build the **library**: a small state-management kit that any app could use, and then you prove it by writing an app on top of it.

It comes in two layers. At the bottom is an **event bus**: you describe your events once, as a type that maps each event name to its payload, and from then on every `on` and `emit` is checked against that description. A typo in an event name, or the wrong payload, is a compile error, not a listener that silently never fires. On top of the bus sits a **store**: it owns some state of whatever shape you give it, changes it only through actions you define as a discriminated union, and announces every change through its own typed events. Add field watchers, partial patches and undo, and you have the core of Redux or Zustand in a couple of hundred lines.

The last milestone is a terminal shopping cart driven by typed commands. The cart is printed because the store said it changed, not because the command loop remembered to print it.

This is the least guided project so far. The milestones are **user stories**: they say what must work and how you'll know, never which files, functions or signatures to write. You also **write your own tests**, including type-level tests: lines that *must not* compile, which `npm run check` verifies. Deciding what to test is part of designing an API.

**What you'll practise:** generic classes and methods, constraints, `keyof` and indexed access types, mapped types, `Partial` and `Readonly`, discriminated unions with exhaustive checks, private fields and custom errors, and testing an API's types as well as its behaviour.

**How to start:** copy the starter with the command on this page, run `npm install`, read `README.md` (it shows the test syntax, including `@ts-expect-error`), and start at milestone 1.

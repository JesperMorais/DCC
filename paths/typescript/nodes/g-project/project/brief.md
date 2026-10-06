In the boss you built a store for one music player, against tests someone else wrote. This time you build the **library**: a small state-management kit that any app could use, you write every test yourself, and then you prove it by writing an app on top of it.

You start on familiar ground. Milestone 1 is the boss store again, rebuilt in your own editor, with the boss lab's checks ported into tests of your own. The code is something you've already made work, so all your attention can go to the new skill: deciding what to test and writing it down.

Then it grows two layers. Underneath is an **event bus**: you describe your events once, as a type that maps each event name to its payload, and from then on every `on` and `emit` is checked against that description. A typo in an event name, or the wrong payload, is a compile error, not a listener that silently never fires. Your store then moves on top of the bus and announces every change through its own typed events, and a rejected action becomes an answer instead of a crash. Add field watchers, partial patches and undo, and you have the core of Redux or Zustand in a couple of hundred lines.

The last two milestones are a terminal shopping cart: first its rules as plain functions you can test without a terminal, then the thin CLI that wires them to stdin. The cart is printed because the store said it changed, not because the command loop remembered to print it.

This is the least guided project so far. The milestones are **user stories**: they say what must work and how you'll know, never which files, functions or signatures to write. You also **write your own tests**, including type-level tests: lines that *must not* compile, which `npm run check` verifies. Deciding what to test is part of designing an API.

**What you'll practise:** generic classes and methods, constraints, `keyof` and indexed access types, mapped types, `Partial` and `Readonly`, discriminated unions with exhaustive checks, private fields and custom errors, and testing an API's types as well as its behaviour.

**How to start:** copy the starter with the command on this page, read `README.md` (the test syntax, a toolbox, and what to do when you're stuck), open your boss lab solution next to it, and start at milestone 1.

This project is long, and that's the point: it's where the pieces from the lessons turn into something you can do on your own. Expect to be stuck more than once, and treat each time as the actual work rather than a delay. One small test, one small change, run it, commit when it's green.

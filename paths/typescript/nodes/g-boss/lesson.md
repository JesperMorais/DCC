Every big frontend ends up with the same problem: ten components need the same data, and each one keeps its own copy. The cart badge says 3, the checkout page says 2. The fix the industry settled on is a **store**: one object that owns the state, changes it only through well-defined actions, and tells everyone when it changed. Redux, Zustand, Pinia and NgRx are all variations of it.

In this boss you build one, and it uses every idea from the Generics section. Here's how they fit together before you start.

### The pieces, and where you learned them

```
             dispatch(action: A)              subscribe(listener)
UI  ───────────────────────────►  Store<S, A>  ─────────────────────► UI
     update(patch: Partial<S>)    #state: S          listener(state)
     select<K extends keyof S>    #reducer(S, A) → S
```

- **Generic class (`Store<S, A>`).** From *Classes & errors*. `S` and `A` are fixed when you write `new Store(initial, reducer)`, inferred from the arguments, so the same class serves a music player, a cart or a counter.
- **Constraints.** From *Generic constraints & keyof*. `A extends { type: string }` says "any action, as long as it has a string `type`". `S extends object` rules out `new Store(42, …)`. Remember the rule of thumb: constrain what you need, and pass the caller's exact type through.
- **`keyof` and `T[K]`.** `select<K extends keyof S>(key: K): S[K]` gives `select("volume")` the type `number`, and makes `select("colour")` a compile error.
- **Utility types.** From *Utility & mapped types*. `update(patch: Partial<S>)` accepts any subset of fields, and `Partial` catches typos like `volum` because a fresh object literal can't have unknown properties. The `state` getter returns `Readonly<S>`, so components can't assign to it.
- **Discriminated unions and `never`.** From *Discriminated unions & never*. `PlayerAction` is tagged by `type`, the reducer `switch`es on it, and `assertNever` makes a forgotten action a compile error.
- **Private fields and custom errors.** The store's `#state` is invisible outside the class. A bad action throws an `InvalidActionError` that callers can catch with `instanceof`, separately from real bugs.

### Two rules that make a store trustworthy

**Never mutate, always replace.** Each change builds a new state object with spread. That way, a component holding last second's `state` still sees last second's values, and "did anything change?" is a cheap `===` check.

**Compute first, commit second.** In `dispatch`, call the reducer *before* assigning to `#state`:

```ts
const next = this.#reducer(this.#state, action); // may throw
this.#state = next;                              // only reached on success
```

If the reducer throws, the line that stores the result never runs. The state is untouched and no listener is told about a change that didn't happen. You get that for free from the order of two lines.

### Gotchas for this lab

- **Types are erased.** `Readonly<S>` stops *TypeScript* code from assigning. It doesn't freeze the object, which is why the immutability rule matters at runtime too.
- **Unsubscribing by identity.** Keep the exact listener function and filter it out with `!==`. Two identical-looking arrow functions are different objects.
- **`Number.isInteger`** rejects `12.5`, `NaN` and `Infinity` in one call, which is what "a whole number" needs.

### In the wild

- **Redux** is exactly `dispatch(action)` → `reducer(state, action)` → notify subscribers, with discriminated unions for actions in typed codebases.
- **Zustand** is a store with `setState(partial)` merging a patch, like your `update`.
- **Pinia and NgRx** expose `select`-style accessors typed with `keyof`, and treat state as read-only from components.

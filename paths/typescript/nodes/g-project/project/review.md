# How we'd structure it

Your layout is as valid as ours if your tests pass and the types say what you mean. Here is one way to do it, and the decisions worth comparing with yours.

## Module layout

```
src/bus.ts     EventBus<E>: knows nothing about stores
src/store.ts   Store<S, A>, its event map, InvalidActionError
src/cart.ts    catalog, CartState, CartAction, cartReducer, total, parseCommand, render
src/main.ts    wiring only: create the store, subscribe printers, read lines
tests/         one file per milestone, plus a tiny counter domain used to test the store
```

The dependency arrows only point one way: `main → cart → store → bus`. The store tests use a counter, not the cart, which is the easiest way to prove the store is really generic.

## Key types

The whole bus hangs off one event map and the indexed access type:

```ts
type ListenerMap<E> = { [K in keyof E]?: Listener<E[K]>[] };

emit<K extends keyof E>(event: K, payload: E[K]): void
```

The mapped type means `#listeners.message` holds listeners for *message* payloads specifically, so no casts are needed inside the class either.

The store's events are just another event map, generic over the store's own parameters, and the cause of a change is a discriminated union:

```ts
type Cause<S, A> =
  | { kind: "action"; action: A }
  | { kind: "patch"; patch: Partial<S> }
  | { kind: "undo" };
```

We tagged it with `kind`, not `type`, so it can't collide with the actions' own `type` field.

## Decisions that matter

**1. Compose the bus, don't extend it.** The store keeps the bus in a private field and exposes an `on` that only forwards. If `Store` extended `EventBus`, anyone could `emit("change", …)` with a fake state, and the "only the store announces changes" guarantee would be gone. The cost is one forwarding method.

**2. One commit path.** Actions, patches and undo all end in the same private method that sets the state and emits `change`. No-op detection (`next === state`), history and notifications live in one place, so they can't drift apart. Field watchers are built *on top of* `change` rather than being a second notification mechanism, so they get undo for free.

**3. Rejections are values, bugs are exceptions.** `dispatch` returns `false` and emits `rejected` for an `InvalidActionError`, and rethrows anything else. The CLI can then print "error: …" without a `try` around every command, while a `TypeError` in a reducer still crashes loudly. The trade-off is that a caller who ignores the boolean doesn't notice the rejection, which is why the event exists.

Smaller choices: `update` bypasses the reducer, so it can't validate. That's fine for UI-only fields, but a shop would probably not allow patching `lines`. Undo keeps every previous state, which is cheap because unchanged parts are shared between states, but a long-running app would cap the history. Prices are whole cents and only become `$1.50` in `render`.

## Testing types

Each test file ends with a function that is never called, full of `// @ts-expect-error` lines. They're tests too: if someone loosens `select` to take any `string`, `npm run check` fails with an unused directive. Write one *positive* line next to the negatives (for example, a `select("count")` assigned to a `number`), so you know the API isn't simply rejecting everything.

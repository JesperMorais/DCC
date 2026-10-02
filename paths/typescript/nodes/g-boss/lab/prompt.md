Your music app's UI keeps all player state in one **store**: components read from it, send it actions, and get told when it changes. Build a generic store class, then the reducer for the player.

**1. `InvalidActionError`** extends `Error`.
`new InvalidActionError(action, reason)` has the message `"<action.type>: <reason>"`, the `name` `"InvalidActionError"`, and a read-only `action` field holding the action. For example: `"pause: Nothing is playing"`.

**2. `playerReducer(state, action)`** returns the **next** `PlayerState` for a `PlayerAction` (see the starter). It never modifies `state`.

| Action | Next state |
|---|---|
| `{ type: "play", track }` | `track` set, `status: "playing"` |
| `{ type: "pause" }` | `status: "paused"`. If the status isn't `"playing"`, throw `InvalidActionError` with reason `Nothing is playing` |
| `{ type: "stop" }` | `track: null`, `status: "stopped"` |
| `{ type: "setVolume", volume }` | `volume` set. If it isn't a whole number from 0 to 100, throw `InvalidActionError` with reason `Volume must be 0-100` |

The `switch` must be exhaustive, with `assertNever` (in the starter) in the `default`.

**3. `Store<S, A>`** is a generic class. `S` is any object type and `A` is any action type with a string `type` field.

| Member | Behaviour |
|---|---|
| `constructor(initial: S, reducer: (state: S, action: A) => S)` | |
| `state` (getter) | The current state, typed `Readonly<S>`. |
| `select(key)` | The value of one field. `key` must be a key of `S`, and the return type is that field's type. |
| `dispatch(action: A)` | Runs the reducer and stores the result. |
| `update(patch: Partial<S>)` | Merges the patch into the state (no reducer). |
| `subscribe(listener)` | Calls `listener(newState)` after every successful `dispatch` or `update`, in subscribe order. Returns an `unsubscribe` function. |

Rules:

- Every change creates a **new** state object. Earlier `state` snapshots and the `initial` object never change.
- If the reducer throws, the error reaches the caller, the state stays as it was, and no listener is called.

```ts
const player = new Store(initialPlayer, playerReducer);
player.dispatch({ type: "play", track: "Dancing Queen" });
player.select("status");        // "playing"
player.update({ volume: 30 });
player.dispatch({ type: "rewind" }); // ✗ type error
player.update({ volum: 30 });        // ✗ type error
```

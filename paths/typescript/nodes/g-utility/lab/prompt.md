Your editor app stores per-user settings (see `Settings` in the starter). Most fields are optional, because users only save what they changed. Instead of hand-writing three more interfaces that will drift out of sync, **derive** them from `Settings`.

**Types**

1. **`Resolved<T>`**: a mapped type you write yourself (not with `Required` or `Readonly`). Every property of `T` becomes **required** and **readonly**. `Resolved<Settings>["fontSize"]` is `number`, not `number | undefined`.
2. **`SettingsPatch`**: what a user may send to change their settings. Every field is optional, and `userId` is not allowed at all.
3. **`PublicSettings`**: a `Resolved<Settings>` without `apiToken`, safe to send to the browser.

Build 2 and 3 from `Settings` and `Resolved` with utility types, not by listing fields.

**Functions** (none of them may modify their input)

4. **`resolve(settings)`** returns a `Resolved<Settings>`: missing fields (absent or `undefined`) are filled from `DEFAULTS`.
5. **`applyPatch(settings, patch)`** returns new `Settings` with the patch's fields replacing the old ones. A field whose value in the patch is `undefined` is **ignored**, not copied over.
6. **`toPublic(settings)`** returns a `PublicSettings`. The returned object must **really not contain** `apiToken`, not just in its type.

```ts
resolve({ userId: "u1", apiToken: "tok", fontSize: 18 });
// { userId: "u1", apiToken: "tok", theme: "light", fontSize: 18, autosave: true }

applyPatch(settings, { theme: "dark", fontSize: undefined }); // theme changes, fontSize stays
applyPatch(settings, { userId: "u2" });                       // ✗ type error
```

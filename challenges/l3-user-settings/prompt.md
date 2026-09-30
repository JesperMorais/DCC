Users can customise the app, but they rarely set everything. Saved settings come back from the database with any unset field simply **missing**.

Write `resolveSettings(saved)` that returns a complete `ResolvedSettings` object, filling in defaults for anything that wasn't set:

| Setting | Default |
|---|---|
| `theme` | `"light"` |
| `fontSize` | `14` |
| `emailAlerts` | `true` |

A value the user **did** set must always be kept — including `false`.

```ts
resolveSettings({});
// { theme: "light", fontSize: 14, emailAlerts: true }

resolveSettings({ theme: "dark", fontSize: 18 });
// { theme: "dark", fontSize: 18, emailAlerts: true }

resolveSettings({ emailAlerts: false });
// { theme: "light", fontSize: 14, emailAlerts: false }
```

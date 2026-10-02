You're writing the timer for a kitchen display. Cooks start a timer in several ways, so the input is a **union**. The two time-picker shapes are in the starter.

**Step 1: the types.** Replace the two placeholders:

- `Preset` is exactly one of `"soft-egg"`, `"hard-egg"` or `"pasta"`.
- `TimerInput` is any one of:
  - a `number`: a number of seconds
  - a `Preset`
  - a `ShortTime`, like `{ minutes: 3, seconds: 30 }`
  - a `LongTime`, like `{ hours: 1, minutes: 15 }`

Anything else, such as `"spaghetti"` or `{ minutes: 3 }`, must be a type error.

**Step 2: the functions.**

**`timerSeconds(input)`** returns the total length in seconds:

| Input | Seconds |
|---|---|
| a number | that number |
| `"soft-egg"` / `"hard-egg"` / `"pasta"` | 360 / 600 / 540 |
| a `ShortTime` | `minutes * 60 + seconds` |
| a `LongTime` | `hours * 3600 + minutes * 60` |

```ts
timerSeconds(90);                         // 90
timerSeconds("pasta");                    // 540
timerSeconds({ minutes: 3, seconds: 30 }); // 210
timerSeconds({ hours: 1, minutes: 15 });   // 4500
```

**`countdownText(remaining)`** formats what the display shows. `remaining` is a number of seconds, or `null` when no timer is running.

- `null` gives `"--:--"`.
- A number gives minutes, a colon, and the seconds as two digits. Minutes are not padded and can go above 59.

```ts
countdownText(null); // "--:--"
countdownText(65);   // "1:05"
countdownText(0);    // "0:00"  (a timer that just finished, not "no timer")
countdownText(4500); // "75:00"
```

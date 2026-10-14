### `return` ends the whole function

Inside a loop, `return` doesn't just skip to the next item. It **leaves the function immediately**, and the rest of the loop never runs:

```ts
function firstLong(words: string[]): string {
  for (const w of words) {
    if (w.length > 5) {
      return w;   // found one: stop everything and hand it back
    }
  }
  return "";      // only reached if the loop finished without finding one
}
```

### "Any" vs "every"

Checking whether **any** item matches is easy to end early: the moment you find one, you know the answer. Checking whether **every** item matches is lopsided: one bad item lets you answer `false` straight away, but you can only answer `true` once you've looked at **all** of them.

### Debugging: trace by hand

Pick a small input and walk through the code on paper, one line at a time. Write down each variable's value and, for a loop, which item you are on. Stop when you hit a `return` and ask: *have I seen enough to be sure of this answer?*

Your music app keeps the play queue as an array of song titles, and the UI keeps the old array around for **undo**. So every helper must **leave the input array exactly as it was** and return a **new** array with the change. (The songs are still added, moved and removed, just in the new array.)

**`upNext(queue, count)`** returns the first `count` songs. If there are fewer, return them all.

**`playNext(queue, song)`** returns a queue with `song` at the front. If the song is already in the queue, it **moves** to the front instead of appearing twice.

**`moveDown(queue, index)`** returns a queue where the song at `index` has swapped places with the one after it. If `index` is the last position or isn't a valid position at all, return an unchanged copy.

```ts
const queue = ["Intro", "Blue", "Echo", "Outro"];

upNext(queue, 2);          // ["Intro", "Blue"]
playNext(queue, "Neon");   // ["Neon", "Intro", "Blue", "Echo", "Outro"]
playNext(queue, "Echo");   // ["Echo", "Intro", "Blue", "Outro"]
moveDown(queue, 1);        // ["Intro", "Echo", "Blue", "Outro"]
moveDown(queue, 3);        // ["Intro", "Blue", "Echo", "Outro"]  (a copy)

queue;                     // still ["Intro", "Blue", "Echo", "Outro"]
```

Type `queue` as `readonly string[]` (the tests pass frozen arrays) and return a `string[]`.

You're working on a music app's playlist screen. A playlist is a `Song[]` (the `Song` type is in the starter). Write four small helpers. The first three should each be a single `return` using `map` and/or `filter`.

**`songTitles(songs)`** returns just the titles, in playlist order:

```ts
songTitles(mix); // ["Hey Jude", "Yesterday", "Bohemian Rhapsody"]
```

**`byArtist(songs, artist)`** returns the songs whose `artist` is exactly `artist`, in playlist order. No match gives `[]`.

```ts
byArtist(mix, "The Beatles"); // [{ title: "Hey Jude", … }, { title: "Yesterday", … }]
```

**`titlesBy(songs, artist)`** returns the *titles* of that artist's songs:

```ts
titlesBy(mix, "Queen"); // ["Bohemian Rhapsody"]
```

**`countMatching(songs, test)`** takes a callback `test` that receives one `Song` and returns a `boolean`. It returns how many songs the callback said `true` for. The callback must be typed, so a callback that reads a property `Song` doesn't have is a type error.

```ts
countMatching(mix, (s) => s.seconds > 200);        // 2
countMatching(mix, (s) => s.title.includes("e"));  // 3
```

None of the functions may change the playlist they were given.

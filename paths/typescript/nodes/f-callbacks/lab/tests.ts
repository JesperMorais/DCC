const mix: Song[] = [
  { title: "Hey Jude", artist: "The Beatles", seconds: 431 },
  { title: "Yesterday", artist: "The Beatles", seconds: 125 },
  { title: "Bohemian Rhapsody", artist: "Queen", seconds: 354 },
];

type callbackCases = [Expect<Equal<Parameters<typeof countMatching>[1], (song: Song) => boolean>>];

// @ts-expect-error — Song has no 'year'
countMatching(mix, (s) => s.year > 2000);

test("songTitles returns the titles in order", () => {
  expect(songTitles(mix)).toEqual(["Hey Jude", "Yesterday", "Bohemian Rhapsody"]);
  expect(songTitles([])).toEqual([]);
});

test("byArtist keeps only that artist's songs", () => {
  expect(byArtist(mix, "The Beatles")).toEqual([
    { title: "Hey Jude", artist: "The Beatles", seconds: 431 },
    { title: "Yesterday", artist: "The Beatles", seconds: 125 },
  ]);
});

test("byArtist with no match is empty", () => {
  expect(byArtist(mix, "ABBA")).toEqual([]);
});

test("titlesBy combines both", () => {
  expect(titlesBy(mix, "Queen")).toEqual(["Bohemian Rhapsody"]);
  expect(titlesBy(mix, "The Beatles")).toEqual(["Hey Jude", "Yesterday"]);
  expect(titlesBy(mix, "ABBA")).toEqual([]);
});

test("countMatching counts what the callback says yes to", () => {
  expect(countMatching(mix, (s) => s.seconds > 200)).toBe(2);
  expect(countMatching(mix, (s) => s.title.includes("e"))).toBe(3);
  expect(countMatching(mix, (s) => s.artist === "ABBA")).toBe(0);
});

test("countMatching calls the callback once per song", () => {
  const seen: string[] = [];
  countMatching(mix, (s) => {
    seen.push(s.title);
    return true;
  });
  expect(seen).toEqual(["Hey Jude", "Yesterday", "Bohemian Rhapsody"]);
});

test("nothing changes the playlist", () => {
  songTitles(mix);
  byArtist(mix, "Queen");
  titlesBy(mix, "Queen");
  expect(mix).toHaveLength(3);
  expect(mix[0]).toEqual({ title: "Hey Jude", artist: "The Beatles", seconds: 431 });
});

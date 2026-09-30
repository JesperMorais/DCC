function shoutAll(words: string[]): string[] {
  return words.map((w) => w.toUpperCase());
}

function longWords(words: string[], minLength: number): string[] {
  return words.filter((w) => w.length >= minLength);
}

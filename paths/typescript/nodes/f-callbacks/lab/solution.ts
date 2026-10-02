type Song = {
  title: string;
  artist: string;
  seconds: number;
};

function songTitles(songs: Song[]): string[] {
  return songs.map((s) => s.title);
}

function byArtist(songs: Song[], artist: string): Song[] {
  return songs.filter((s) => s.artist === artist);
}

function titlesBy(songs: Song[], artist: string): string[] {
  return byArtist(songs, artist).map((s) => s.title);
}

function countMatching(songs: Song[], test: (song: Song) => boolean): number {
  let count = 0;
  for (const song of songs) {
    if (test(song)) count++;
  }
  return count;
}

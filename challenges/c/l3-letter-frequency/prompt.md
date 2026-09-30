Frequency analysis is how people crack simple substitution ciphers: in English, the most common letter is usually `e`. Write `letter_counts`, which tallies how often each letter appears in a string:

```c
void letter_counts(const char *text, int counts[26]);
```

- `counts[0]` is the number of `a`/`A`, `counts[1]` the number of `b`/`B`, … `counts[25]` is `z`/`Z`.
- Upper- and lowercase count as the same letter.
- Everything that isn't a letter (digits, spaces, punctuation) is ignored.
- **Overwrite** all 26 entries: the caller doesn't have to zero the array first.

```c
int counts[26];
letter_counts("Hello!", counts);
// counts['h' - 'a'] == 1, counts['e' - 'a'] == 1, counts['l' - 'a'] == 2, counts['o' - 'a'] == 1, everything else 0
```

Assume plain ASCII text.

A music library imports song titles in every possible casing. Write `capitalize_words`, which rewrites a string **in place** so that the first character of every word is uppercase and the rest of the word is lowercase:

```c
void capitalize_words(char *s);
```

A word is a run of non-whitespace characters (use `isspace`). Whitespace itself is left exactly as it is, and so are non-letters.

```c
char title[] = "bOHEMIAN   rhapsody";
capitalize_words(title);   // title is now "Bohemian   Rhapsody"
```

- `"hey jude"` → `"Hey Jude"`
- `"99 red balloons"` → `"99 Red Balloons"` (the first "letter" of `99` is a digit, and stays one)
- `""` → `""`

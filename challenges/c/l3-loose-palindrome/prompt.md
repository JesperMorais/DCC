A word-game app lets players submit palindromes, and it's forgiving about spelling style. Write `is_loose_palindrome`, which returns `true` if the **letters** of the string read the same forwards and backwards, ignoring case and ignoring every character that isn't a letter (spaces, punctuation, digits):

```c
bool is_loose_palindrome(const char *s);
```

- `"A man, a plan, a canal: Panama!"` → `true`
- `"Racecar"` → `true`
- `"palindrome"` → `false`
- `""` → `true`, and so is a string with no letters at all, like `"?! 42"`

Don't copy the string: walk it from both ends with two indices.

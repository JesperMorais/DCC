### Where `strlen` lands you

A C string is a run of characters followed by a `'\0'`. `strlen(s)` counts the characters *before* the terminator, so `s + strlen(s)` points **at the terminator itself**, one past the last real character:

```c
char s[] = "hi\n";          // 'h' 'i' '\n' '\0'
char *end = s + strlen(s);  // points at '\0' (index 3)
end[-1];                    // '\n', the last real character
```

That is the same half-open idea as `begin`/`end` ranges: `end` marks the boundary, and the last element lives at `end - 1`. Walking backwards means moving `end` down while the character *before* it matches, and stopping at `begin` so you never step in front of the array.

### Debugging: trace the boundaries by hand

When a loop "does nothing", write a three-character input on paper with its indices, then step through the code: what does each pointer point at *before* the first iteration? What does the condition read? Most pointer bugs show up in the first or last step, not the middle.

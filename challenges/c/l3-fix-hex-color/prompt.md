**Bug report from the design team:** "When I paste `#FF0000` into the theme editor, the preview turns a dull dark red instead of bright red. `#336699` looks right though."

`hex_pair_value` reads one colour channel: two hexadecimal characters such as `"FF"` or `"3c"`, and returns their value from `0` to `255`:

```c
int hex_pair_value(const char *pair);
```

- `pair` points at (at least) two characters; anything after the first two is ignored.
- Hex digits are `0`–`9` and `A`–`F`, in **upper or lower case**, where `A` = 10 … `F` = 15.
- The value is `first_digit * 16 + second_digit`.
- If either character is not a hex digit, return `-1`.

What it should do:

- `hex_pair_value("33")` → `51`
- `hex_pair_value("c0")` → `192`
- `hex_pair_value("G7")` → `-1`

**Reproduction:** `hex_pair_value("FF")` returns `85`, expected `255`.

Find the bug and fix it.

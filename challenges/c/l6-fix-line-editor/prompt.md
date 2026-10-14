**Bug report from the serial console**

> The command line on our debug console has basic editing: arrow keys, insert, delete. Typing at the end of the line is fine, but if I move the cursor back to fix a typo, the rest of the line turns to garbage. `helo` with an `l` inserted becomes `helloo`.

`EditLine` holds one line of text (at most `LINE_CAP - 1` characters, always NUL-terminated) and its length.

- `line_set(line, s)` replaces the contents with `s`, truncated to fit.
- `line_insert(line, pos, s)` inserts `s` before index `pos` (`pos == len` appends). It returns `false` and changes nothing if `pos > len` or the result wouldn't fit.
- `line_erase(line, pos, count)` removes up to `count` characters starting at `pos` (fewer if the line ends first). It returns `false` if `pos > len`.
- `line_replace(line, pos, count, s)` erases like `line_erase` and inserts `s` at `pos`. It returns `false` and changes nothing if `pos > len` or the result wouldn't fit.

```c
line_set(&l, "temp=");      line_insert(&l, 5, "21.5");    // "temp=21.5"
line_set(&l, "reboot now"); line_erase(&l, 6, 4);          // "reboot"
line_set(&l, "a=1;b=2");    line_replace(&l, 2, 1, "10");  // "a=10;b=2"
```

**Reproduction**

```c
EditLine l = {0};
line_set(&l, "helo");
line_insert(&l, 3, "l");
// actual:   l.text = "helloo"
// expected: l.text = "hello", l.len = 5
```

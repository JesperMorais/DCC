**Bug report from the log ingester team**

> We read log lines with `fgets`, so every line arrives with its `"\n"` (or `"\r\n"` from the Windows boxes) still attached, and sometimes trailing spaces too. `chomp_line` is supposed to strip all of that, but our dashboards still show `"disk full\r"`. As far as we can tell, it never strips anything.

`chomp_line` removes **trailing** whitespace (spaces, tabs, `\r`, `\n`, anything `isspace` accepts) from a string **in place** and returns the new length. Whitespace at the start or in the middle stays.

```c
char a[] = "disk full\r\n";   chomp_line(a);   // a = "disk full", returns 9
char b[] = "  indented  \t";  chomp_line(b);   // b = "  indented", returns 10
char c[] = "\n";              chomp_line(c);   // c = "", returns 0
```

**Reproduction**

```c
char line[] = "disk full\r\n";
size_t n = chomp_line(line);
// actual:   n = 11, line = "disk full\r\n"
// expected: n = 9,  line = "disk full"
```

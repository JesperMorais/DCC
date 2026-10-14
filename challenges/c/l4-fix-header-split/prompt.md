**Bug report from the proxy team**

> Our little HTTP proxy splits each request header with `split_header`. Most headers are fine, but requests to non-default ports get routed to the wrong host, and the `Referer` we log is garbage.

`split_header` splits one header line in place into a name and a value:

- The name is everything before the **first** `:`. It must not be empty.
- The value is everything after that colon, with leading spaces skipped. It may be empty.
- On success it writes the `:` over with `'\0'`, points `*name` and `*value` into `line` and returns `true`.
- If there's no colon, or the name is empty, it returns `false` and doesn't touch `line`, `*name` or `*value`.

```c
"Host: example.com"   →  true, name "Host", value "example.com"
"Content-Length:42"   →  true, name "Content-Length", value "42"
"no colon here"       →  false
```

**Reproduction**

```c
char line[] = "Host: example.com:8080";
char *name, *value;
split_header(line, &name, &value);
// actual:   name = "Host: example.com", value = "8080"
// expected: name = "Host",              value = "example.com:8080"
```

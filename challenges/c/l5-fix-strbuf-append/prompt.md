**Bug report from the telemetry service**

> We build our JSON payloads with `StrBuf`. It ran fine for weeks, then a sensor with a long serial number came online and the sanitizer build started crashing in `strbuf_append`.

`StrBuf` is a growable, always NUL-terminated string:

- `strbuf_init` makes an empty builder that owns no memory yet (`data` is `NULL`).
- `strbuf_append(sb, text)` appends `text`, growing the buffer as needed by doubling its capacity (starting at 16), and returns `true`. If memory runs out it returns `false` and leaves the builder as it was.
- After an append, `sb->data` holds the whole string so far and `sb->len` its length.
- `strbuf_free` releases the memory and leaves an empty builder.

```c
StrBuf sb;
strbuf_init(&sb);
strbuf_append(&sb, "{\"id\":");   // data "{\"id\":", len 6
strbuf_append(&sb, "42}");        // data "{\"id\":42}", len 9
strbuf_free(&sb);
```

**Reproduction**

```c
StrBuf sb;
strbuf_init(&sb);
strbuf_append(&sb, "serial=SN-2026-0042-ABCDEFGH-REV-C");
// actual:   AddressSanitizer: heap-buffer-overflow
// expected: true, len 34
```

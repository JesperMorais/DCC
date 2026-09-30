A config loader reads `port=8080` style settings, and you're writing the bit that turns the text after `=` into a number, without `atoi` (which can't report errors). Write `parse_port`:

```c
bool parse_port(const char *text, unsigned *out);
```

- `text` must be **one or more decimal digits and nothing else**: no sign, no spaces, no trailing junk.
- The value must be a valid TCP port, `0`–`65535`. Leading zeros are fine (`"080"` is 80).
- On success, store the value in `*out` and return `true`.
- On failure, return `false` and **leave `*out` unchanged**.

```c
unsigned port = 0;
parse_port("8080", &port);    // true, port == 8080
parse_port("65536", &port);   // false: too big
parse_port("80a", &port);     // false: not all digits
parse_port("", &port);        // false: no digits
```

Don't use `atoi`, `strtol`, `sscanf` or friends. Parse the digits yourself.

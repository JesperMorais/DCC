### Where does a C string end?

A C string is just an array of `char`. Nothing records its length: by convention, the string ends at the first `'\0'` byte (the **null terminator**). `printf`, `strlen`, `strcmp` all walk forward until they hit one.

```c
char buf[8] = "cat";     // c a t \0 \0 \0 \0 \0
buf[0] = 'b';            // b a t \0 ...   → "bat"
buf[3] = 's';            // b a t s \0 ... → "bats"
```

Writing characters into a buffer doesn't make a string by itself. Whatever is in the bytes **after** your characters still counts until the first `'\0'`. A buffer that's freshly zeroed hides this; a buffer that's been used before doesn't.

When you copy into a fixed-size buffer of `cap` bytes, the longest string that fits has `cap - 1` characters, because the terminator needs a byte too.

### Debugging: draw the bytes

For string bugs, draw the buffer as a row of boxes, one per byte, **including its old contents**:

```
before:  a d m i n i s t r a t o r \0 \0 \0
```

Then run the code by hand and change the boxes it writes to. When you're done, read from the left until the first `\0`. That's what everyone else will see.

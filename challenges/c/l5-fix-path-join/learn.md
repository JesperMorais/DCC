### Sizing a buffer: count every byte you'll write

`malloc(n)` gives you exactly `n` bytes, and writing byte `n` is already out of bounds. Without a sanitizer, a one-byte overflow usually doesn't crash; it quietly overwrites the allocator's bookkeeping or a neighbouring object, and something unrelated breaks later. That's why "works in release" proves nothing.

The reliable way to size a string buffer is to list everything you will write, in order, and add it up:

```c
// "<user>@<host>"
size_t size = strlen(user)   // the user
            + 1              // the '@'
            + strlen(host)   // the host
            + 1;             // the '\0'
```

Pieces that are only written sometimes count in exactly the cases where they are written.

### Debugging: let the sanitizer point, then count

An out-of-bounds report tells you the line that made the bad **access**, but the mistake is usually somewhere else: in the line that decided how big the block would be. Take the input that fails, write down every byte the function puts into the buffer, and compare that count with the number passed to `malloc`. If they differ by one, look for the piece you wrote but never counted.

### Moving bytes inside one buffer

Inserting or deleting in the middle of an array means shifting everything after that point. When the source and destination of a copy are **the same buffer**, the order you copy in matters:

```
shift "abcd" right by 1, copying left to right:
  a b c d _      i=0: copy a → slot 1
  a a c d _      i=1: copy slot 1 → slot 2  (but slot 1 is already overwritten!)
  a a a d _      ...
```

- Moving data **left** (towards lower indices), copy from left to right: every slot is read before it's overwritten.
- Moving data **right**, copy from right to left, for the same reason.

The standard library knows this rule: `memcpy` requires that the regions don't overlap, while `memmove` handles overlap correctly in both directions. Hand-written loops have to get the direction right themselves.

A backwards loop over `size_t` needs care, because `i >= 0` is always true for an unsigned type. The usual idiom tests and decrements in one go: `for (size_t i = n; i-- > 0;)`.

### Debugging: print the buffer every iteration

When a loop rearranges data in place, print the whole buffer after each iteration of a small example (four or five characters is plenty). The step where the output first looks wrong tells you which read happened after a write it shouldn't have seen.

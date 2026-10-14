### A string is reached through a pointer

In C a "string" variable is really a `const char *`: the **address** of the first character. That makes two very different questions easy to mix up:

```c
const char *a = "reset";
char b[] = "reset";

a == b;              // same address? (false: b is its own array)
strcmp(a, b) == 0;   // same characters? (true)
```

`==` on pointers asks "do these point at the same place in memory?". That's the right question for identity checks like `p == NULL` or `p != end`, and the wrong one for comparing text.

The compiler is allowed to store identical string literals only once, so `"reset" == "reset"` is often true by luck. Code that compares addresses can pass every test written with literals and then fail on text that arrived at runtime (typed input, a file, a network packet).

### Debugging: print the addresses

When a comparison you're sure should succeed doesn't, print both sides, including the pointer values:

```c
printf("%p '%s' vs %p '%s'\n", (void *)a, a, (void *)b, b);
```

Same text, different addresses? Then you know what `==` was looking at.

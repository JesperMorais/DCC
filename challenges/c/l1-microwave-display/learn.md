### Division with whole numbers

In C, the **type** of a value decides how maths works on it. An `int` can only hold whole numbers, so when you divide one `int` by another, the answer is also a whole number. C simply **drops** whatever doesn't fit:

```c
int eggs = 30;
int full_cartons = eggs / 12;   // 2   (not 2.5!)
int loose_eggs   = eggs % 12;   // 6
```

- `/` answers "how many **whole** times does 12 fit into 30?" (2)
- `%` (say "modulo" or "remainder") answers "what's **left over**?" (30 − 2 × 12 = 6)

Together they split a number into "full groups" and "the rest". That's exactly what you need for dozens, hours and minutes, or pages of 10 search results.

### Variables are named boxes

`int full_cartons = eggs / 12;` makes a new **variable** called `full_cartons` of type `int` and stores the result in it. You can use it later in the function, which is handy for breaking a calculation into readable steps instead of one long line.

### A quick check

`7 % 2` is `1` and `8 % 2` is `0`. A remainder of 0 means "divides evenly", which is how you test for even numbers.

### Characters are numbers

In C, a `char` is a small integer: `'0'` is 48, `'A'` is 65, `'a'` is 97. The digits are next to each other, and so are the letters, which lets you do arithmetic on them:

```c
int digit = '7' - '0';         // 7, because '7' is 55 and '0' is 48
char next = 'c' + 1;           // 'd'
int position = 'E' - 'A';      // 4: E is the 5th letter, counting from 0
```

Subtracting the **first** character of a range gives the position **inside that range**, starting from 0. Whether that position is the value you want depends on what the range stands for.

In hexadecimal (base 16), the digits run `0 1 2 … 9 A B C D E F`, so `A` is worth 10 and `F` is 15. A two-digit hex number works like a decimal one, only with 16 instead of 10: `"2A"` is `2 * 16 + 10 = 42`.

### Debugging: test each piece on its own

When a function is built from a helper, test the **helper** first with a handful of inputs and write down what you expect for each: `'0'`, `'9'`, `'A'`, `'f'`, `'G'`. A wrong value at that level is much easier to spot than a wrong final answer, where two small errors get mixed together.

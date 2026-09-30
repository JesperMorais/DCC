### In place means relinking, not copying

A linked list's order lives entirely in its `next` pointers. The nodes themselves can stay exactly where they are in memory. You reorder the list by changing which node each one points at.

That's cheaper than building a copy (no allocations, O(1) extra memory), and it keeps any outside pointers to individual nodes valid.

### The danger: losing the rest of the list

In a singly linked list, the **only** way to reach node 3 is through node 2's `next`. Overwrite that field without saving it first and everything after it becomes unreachable. That's a leak, and a bug.

```c
// Moving the front node of `src` onto the front of `dst`:
Item *moving = src;
src = src->next;        // 1. save the rest of src first
moving->next = dst;     // 2. now it's safe to overwrite
dst = moving;
```

When a loop rewires pointers, write down on paper what each pointer holds at the start of an iteration. Most bugs show up as "I overwrote something I still needed".

### Test your edge cases

Run your logic by hand on an empty list, one node and two nodes. Loops that are correct for five nodes are often wrong for zero.

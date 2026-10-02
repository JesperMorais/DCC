Your warehouse packs mugs into boxes that each hold the same number of mugs. Write three small helpers for the packing screen. Each takes the number of `items` in the order and how many fit `perBox`, both whole numbers, and returns a number.

**`fullBoxes(items, perBox)`** is how many boxes can be filled completely.

**`leftover(items, perBox)`** is how many items are left over after filling those full boxes.

**`boxesNeeded(items, perBox)`** is how many boxes you need to ship *everything*. A partly filled box still counts as a box.

```ts
fullBoxes(14, 6);   // 2
leftover(14, 6);    // 2
boxesNeeded(14, 6); // 3   (6 + 6 + 2)

fullBoxes(12, 6);   // 2
leftover(12, 6);    // 0
boxesNeeded(12, 6); // 2

boxesNeeded(0, 6);  // 0   (nothing to ship)
```

Annotate the parameters and return types as `number`, so that `fullBoxes("14", 6)` is a type error.

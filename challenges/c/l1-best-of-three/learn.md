### Variables can change

A **variable** is a named box that holds one value of a fixed **type**. You create it once, and you can put a new value in it whenever you like:

```c
int lives = 3;      // create the box and put 3 in it
lives = lives - 1;  // take out 3, subtract 1, put 2 back
```

The `int` only appears when the box is **created**. After that you just use its name. C insists on the type up front so it knows how much memory the box needs and what kind of values are allowed in it.

### Making decisions

An `if` runs a block of code only when its condition is true:

```c
int ticket_price(int age) {
    int price = 120;
    if (age < 12) {
        price = 60;      // children pay half
    }
    if (age >= 65) {
        price = 80;      // senior discount
    }
    return price;
}
```

Each `if` is checked **on its own**, one after the other. A variable that you update along the way is a great way to build up an answer step by step.

Comparison operators: `<`, `>`, `<=`, `>=`, `==` (equal; note the **two** equals signs) and `!=` (not equal).

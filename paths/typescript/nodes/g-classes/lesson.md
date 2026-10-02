A checkout service wraps its payment call in a `try/catch` and shows "Card declined, try another card" on any error. One day a developer renames a variable and leaves a typo behind. Now *every* checkout throws `ReferenceError`, the catch turns it into "Card declined", and for two hours the support team tells customers their cards are broken.

The bug wasn't the `try/catch`. It was catching **everything** the same way. This lesson covers classes, how to keep their data private, and how to throw errors that the caller can tell apart.

### A class bundles data with the code that guards it

You've used objects and functions separately. A **class** is a blueprint for objects that carry both:

```ts
class Counter {
  count = 0;                    // a field: every Counter gets its own

  increment(): void {           // a method
    this.count += 1;            // `this` is the object the method was called on
  }
}

const clicks = new Counter();   // `new` builds an object from the blueprint
clicks.increment();
clicks.count; // 1
```

A **constructor** runs once, inside `new`, to set the object up:

```ts
class Account {
  owner: string;
  balance = 0;

  constructor(owner: string) {
    this.owner = owner;
  }
}
const acc = new Account("Ada");
```

### Private fields: only the class may touch them

Right now anyone can write `acc.balance = 1_000_000`. The point of a class is that its methods enforce the rules, so the data must be off-limits from outside. TypeScript gives you two ways to do that:

```ts
class Account {
  private balance = 0;  // TypeScript-only: a compile error outside, but a normal property at runtime
  #pin: string;         // JavaScript private field: truly hidden, even at runtime

  constructor(pin: string) {
    this.#pin = pin;
  }
}

acc.balance; // ✗ Property 'balance' is private
acc.#pin;    // ✗ not accessible outside class 'Account'
```

Remember that types are erased. `private` is only checked by the compiler, so `(acc as any).balance` still works at runtime, and the field shows up in `console.log`. A `#field` is enforced by JavaScript itself. Prefer `#` for new code. Either way, expose what callers may read through a **getter**:

```ts
get balance(): number {   // read as acc.balance, no parentheses
  return this.#balance;
}
```

With a getter and no setter, `acc.balance = 5` is a compile error.

### Generic classes

In *Generics* you wrote `function first<T>(items: T[])`. A class can take a type parameter the same way, and every method can use it:

```ts
class Inbox<T> {
  #items: T[] = [];

  add(item: T): void { this.#items.push(item); }
  next(): T | undefined { return this.#items.shift(); }
}

const mail = new Inbox<string>();
mail.add("hi");  // ✓
mail.add(42);    // ✗ number is not assignable to string
```

`T` is chosen once, when the object is created, and then holds for that object's whole life.

### Errors: throw them, catch them, tell them apart

`throw` stops the function and jumps to the nearest `catch`. Always throw an `Error` object (not a string), because it carries a `message` and a stack trace:

```ts
if (amount > this.#balance) throw new Error("Insufficient funds");
```

To let callers react to *one kind* of failure, make your own error class with `extends`:

```ts
class InsufficientFundsError extends Error {
  readonly shortBy: number;

  constructor(shortBy: number) {
    super(`Insufficient funds: short by ${shortBy}`); // sets .message
    this.name = "InsufficientFundsError";            // shows up in logs
    this.shortBy = shortBy;
  }
}
```

`super(...)` runs the parent `Error`'s constructor, and it must come before you use `this`. Now the caller can catch precisely:

```ts
try {
  acc.withdraw(500);
} catch (err) {
  if (err instanceof InsufficientFundsError) {
    showMessage(`You need ${err.shortBy} more`); // err is narrowed
  } else {
    throw err; // not ours: let it crash loudly
  }
}
```

That `else { throw err }` is the fix for the checkout story. A typo's `ReferenceError` is not a declined card.

### `finally`: cleanup that always runs

```ts
lock.acquire();
try {
  return doWork();
} finally {
  lock.release(); // runs after return, and after a throw too
}
```

### Gotchas

- **`catch (err)` gives you `unknown`.** Anything can be thrown in JavaScript, so you must narrow with `instanceof` before using `err.message`.
- **Forgetting `this.`** Inside a method, `balance` alone is not the field. Write `this.balance` or `this.#balance`.
- **Getters aren't called.** `acc.balance`, not `acc.balance()`.
- **Return copies, not your private array.** `get items() { return this.#items; }` hands callers the real array, and they can `push` into it. Return `[...this.#items]` instead.

### In the wild

- **Node.js** errors carry a `code` (`ENOENT`, `ECONNRESET`), so code can catch "file not found" without catching everything else.
- **Axios** throws an `AxiosError` with the HTTP response attached. `instanceof AxiosError` separates network failures from bugs.
- **Database pools** in `pg` and `mysql2` are classes with private state and `acquire`/`release` methods, used with `try/finally` so connections always go back.
- **Angular and NestJS** services are classes, and their `private` constructor parameters are the same `private` you saw here.

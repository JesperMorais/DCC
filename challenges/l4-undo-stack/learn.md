### Classes with private state

A class bundles data with the methods that are allowed to change it. Hiding the data means nobody can put the object into a broken state from the outside.

```ts
class Counter {
  #count = 0;                 // truly private (JavaScript feature)
  private step = 1;           // private for the type checker (TypeScript feature)

  increment(): void {
    this.#count += this.step;
  }

  get value(): number {       // a getter: read as counter.value
    return this.#count;
  }
}

const c = new Counter();
c.increment();
c.value;    // 1
c.#count;   // Error: not accessible outside the class
```

### Generic classes

Classes take type parameters just like functions. `class Box<T> { constructor(public item: T) {} }` — then `new Box<number>(5)` only accepts numbers.

### Throwing errors

`throw new Error("message")` stops the method immediately. Callers can catch it with `try { ... } catch (e) { ... }`. Throwing is the right move when continuing would mean returning a value you don't actually have.

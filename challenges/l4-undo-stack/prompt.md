Your text editor needs **undo**. Every edit is pushed onto a stack; undo pops the most recent one. Implement a generic class `Stack<T>`:

| Member | Behaviour |
|---|---|
| `push(item: T): void` | Adds an item on top. |
| `pop(): T` | Removes and returns the top item. If the stack is empty, **throw** an `Error` with the message `"Stack is empty"`. |
| `peek(): T \| undefined` | Returns the top item **without** removing it, or `undefined` if empty. |
| `size` (getter) | The number of items: `stack.size`, not `stack.size()`. |

The storage must be **private**: code outside the class must not be able to reach the array — `stack.items` must be a type error.

```ts
const history = new Stack<string>();
history.push("type 'Hello'");
history.push("bold 'Hello'");
history.size;   // 2
history.peek(); // "bold 'Hello'"
history.pop();  // "bold 'Hello'"
history.pop();  // "type 'Hello'"
history.pop();  // throws Error("Stack is empty")
history.push(42); // ✗ compile error — this is a Stack<string>
```

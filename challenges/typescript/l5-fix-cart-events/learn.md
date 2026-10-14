### How `this` is decided

In a normal method, `this` isn't fixed when you write the code. It's whatever is **in front of the dot when the function is called**:

```ts
class Greeter {
  name = "Ada";
  hello() { return `hi ${this.name}`; }
}

const g = new Greeter();
g.hello();               // "hi Ada": called as g.hello(), so this === g

const loose = g.hello;   // just the function, no object attached
loose();                 // TypeError: this is undefined
```

Passing `g.hello` as a callback is the same as `loose`: whoever calls it later calls it as a plain function. (Inside classes, code runs in strict mode, so `this` becomes `undefined` rather than some global object.)

Arrow functions are different: they don't have their own `this` and use the one from where they were **written**. That's why `() => this.something()` inside a constructor keeps working when called later.

### Debugging technique: compare the working path with the broken one

When one route to the same code works and another crashes, line them up side by side. What's different about how the function is reached? Here that's often "called on an object" vs "called on its own".

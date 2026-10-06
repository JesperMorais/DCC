Every developer has written a to-do app at some point, and there's a good reason for it: it's the smallest program that does everything a real one does. It holds data, changes it, shows it to a person, and remembers it after the program has stopped. If you can build a to-do list cleanly, you can build the core of an invoicing tool, a bug tracker or a shopping cart, because they're the same shape with more fields.

You're going to build one that lives in your terminal:

```
$ npm start add buy milk
Added 1. buy milk
$ npm start add call mum
Added 2. call mum
$ npm start done 1
Done: buy milk
$ npm start list
[x] 1. buy milk
[ ] 2. call mum

1 of 2 left
```

Close the terminal, come back tomorrow, and the list is still there, saved in a `todos.json` file you can open and read.

The interesting part isn't the terminal. It's that the heart of the app is a handful of small, **pure** functions: each takes the list and gives back a new one, without ever changing what it was given. That's exactly what Fundamentals has been training you to do. Adding is a copy with one more item, marking done is a `map`, removing is a `filter`, counting is a loop with an accumulator, and the summary line is an `if / else` chain. Only at the very end do you plug them into a file and the command line.

You'll practise arrays without mutation, objects and type aliases, strings, decisions, loops, `map` and `filter`, and something new that every real project needs: splitting a program into files that each do one job.

The signatures and the tests are already written, so you always know what "done" means. What goes inside the functions is up to you.

**How to start:** copy the starter (the command is on this page), run `npm install` in the new folder, and read `README.md` (the Workshop node before this one walks through all of it). Then open `src/todo.ts` and start at milestone 1.

There's no clock on this. Some milestones will click in ten minutes and one might take an evening, and both are normal. When you're stuck, that's the part that's teaching you something: shrink the step, look at the actual values, take a hint if you want one, and commit every time the tests go green.

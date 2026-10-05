### Where did the money go?

Every month the same thing happens: payday, a few weeks of tapping your card, and then a balance that's somehow lower than you expected. Your bank will happily give you a CSV export of everything, but a few hundred lines of `ICA Nara,-89.90` don't answer the question you actually have: **what am I spending it on?**

You're going to build a small command-line tool that does. It reads the bank's export, sorts each transaction into a category using a rules file you write yourself ("anything with *ica*, *coop* or *willys* in it is groceries"), and answers a few questions:

- `report 2026-09`: how much went to each category this month, biggest first, and the total.
- `top 5`: the five largest single expenses, the ones worth looking at twice.
- `months`: money out, money in and the net for every month, so you can see the trend.

Bank exports are not tidy. Some rows have a comma in the amount, a date the bank's own system mangled, or nothing at all. Your tool won't crash on them and won't silently guess. It skips them and tells you exactly which line was wrong and why, so you can trust the numbers it does print.

This is the Core section put to work: interfaces for the data, a union for "a row that worked or a row that didn't" and narrowing to tell them apart, `reduce` and filter/sort/map pipelines to get from rows to totals, a `Map` or `Record` for grouping, regular expressions and string parsing for the CSV and the rules, and optional properties for the flags. The data types are given; the functions are yours to design. The tests run your CLI as a real program and only check what it prints, so any sensible structure passes.

**How to start:** copy the starter (the command is on this page), run `npm install`, read the `README.md`, and start at milestone 1. Each milestone spells out the exact output, and `npm run test:m1` (and so on) tells you when you're there.

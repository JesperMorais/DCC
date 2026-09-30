In an election, a person may vote only if **both** of these are true:

1. they are **18 or older**, and
2. they are a **citizen**.

Write a function `canVote(age, isCitizen)` that returns `true` if the person may vote, otherwise `false`.

- `canVote(20, true)` → `true`
- `canVote(18, true)` → `true` (exactly 18 is old enough)
- `canVote(17, true)` → `false` (too young)
- `canVote(40, false)` → `false` (not a citizen)

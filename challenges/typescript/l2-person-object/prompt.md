**Step 1 — the type.** Replace the placeholder `type Person = {};` with a real object type:
a person has a `name` (a string) and an `age` (a number). Nothing else.

**Step 2 — two functions.**

`describe(person)` returns a sentence:

- `describe({ name: "Ada", age: 36 })` → `"Ada is 36 years old"`

`haveBirthday(person)` returns a **new** person object that is one year older.
Do not change the original object.

- `haveBirthday({ name: "Linus", age: 20 })` → `{ name: "Linus", age: 21 }`

The tests also check that a person without an `age` is rejected by TypeScript.

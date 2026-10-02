A sign-up form kept rejecting people who were sure they'd typed their email right. The check looked fine:

```ts
email.trim();
if (email.endsWith("@example.com")) { … }
```

The bug: `email.trim()` doesn't change `email`. It *returns* a trimmed copy, and that line threw the copy away. Anyone whose phone autocomplete added a trailing space was locked out. Strings are full of small rules like this one, and once you know them they stop biting.

### Writing strings

A `string` is a piece of text. You can write it three ways:

```ts
const a = 'single quotes';
const b = "double quotes";            // same thing
const name = "Ada";
const c = `Hello, ${name}!`;          // template literal: "Hello, Ada!"
```

The backtick version is a **template literal**. Anything inside `${…}` is evaluated and dropped into the text, so ``` `${qty} × ${price}` ``` beats `qty + " × " + price`. Use `+` for strings only when it's simpler.

### Strings are immutable

You can never change a string in place. Every string method **returns a new string** and leaves the original alone:

```ts
let email = "  Ada@Example.com ";
email.toLowerCase();                 // returns a new string… which nobody keeps
email = email.trim().toLowerCase();  // ✓ keep the result
```

Notice the **chaining**: `trim()` returns a string, so you can call `.toLowerCase()` on it directly.

### Indexing: positions start at 0

Each character has a position, its **index**, starting from `0`:

```
 "T  y  p  e"
  0  1  2  3      length = 4, last index = length - 1 = 3
```

```ts
const word = "Type";
word.length;            // 4
word[0];                // "T"
word[word.length - 1];  // "e"   the last character
word[10];               // undefined, not an error!
word.charAt(10);        // ""    charAt gives an empty string instead
```

Reading past the end gives `undefined`, so `word[0].toUpperCase()` *crashes* when `word` is `""`. `charAt(0)` returns `""` for an empty string, which is safe to call methods on. There's no separate "char" type in TypeScript: a single character is just a `string` of length 1.

### The methods you'll use every day

```ts
const s = "  Hello World  ";
s.trim()                    // "Hello World"      remove surrounding spaces
s.toUpperCase()             // "  HELLO WORLD  "
"Hello".includes("ell")     // true               contains?
"report.pdf".endsWith(".pdf")    // true
"https://x".startsWith("https")  // true
"a-b-c".replaceAll("-", " ")     // "a b c"         replace() only does the first
"a,b,c".split(",")               // ["a", "b", "c"]
"7".padStart(3, "0")             // "007"
```

**`slice(start, end)`** cuts out a piece. `end` is *not included*, and negative numbers count from the end:

```
 "TypeScript"
  0123456789
slice(0, 4)  → "Type"     indices 0,1,2,3
slice(4)     → "Script"   from 4 to the end
slice(-6)    → "Script"   the last 6 characters
```

### Worked example: a display name

```ts
function displayName(raw: string): string {
  const clean = raw.trim();
  return clean.charAt(0).toUpperCase() + clean.slice(1).toLowerCase();
}
displayName("  aDA ");  // "Ada"
displayName("");        // ""   (charAt and slice are both safe on "")
```

Each step returns a new string and the next step uses it. The original `raw` is untouched.

### Gotchas

- **Forgetting to keep the result.** `s.trim();` on its own line does nothing. Write `s = s.trim()` or use the result directly.
- **`string`, not `String`.** In annotations, always write lowercase `string`. Capital `String` is the type of the rarely used wrapper object (`new String("x")`), and mixing them up gives confusing errors.
- **`+` with a number glues.** `"3" + 1` is `"31"`, not `4`. TypeScript allows it because joining text is legal, so check that your values really are numbers.
- **`replace` replaces once.** `"a-b-c".replace("-", " ")` is `"a b-c"`. Use `replaceAll`.
- **Comparisons are exact.** `"Milk" === "milk"` is `false`. Lowercase both sides when case shouldn't matter.

### In the wild

- **Every sign-up form** runs `email.trim().toLowerCase()` before saving, so `Ada@x.com ` and `ada@x.com` are the same account.
- **Card payment screens** show `•••• 4242` by slicing the last four digits with `slice(-4)`.
- **Blogs and shops** turn titles like "Summer Sale 2026" into URL slugs like `summer-sale-2026`.
- **Invoice numbers** like `INV-000042` are built with `padStart`.

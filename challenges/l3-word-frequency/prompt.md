A support team wants to know which words show up most in customer feedback. Write `wordFrequency(text)` that returns an object mapping each word to how many times it appears.

Rules:

- Matching is **case-insensitive**: `"Slow"` and `"slow"` are the same word. Keys are lowercase.
- A word is a run of letters `a–z` and digits `0–9`. Everything else (spaces, punctuation, newlines) separates words.
- Text with no words gives `{}`.

```ts
wordFrequency("App is slow. Very slow!");
// { app: 1, is: 1, slow: 2, very: 1 }

wordFrequency("Error 404, error 500");
// { error: 2, "404": 1, "500": 1 }

wordFrequency("  ...  ");
// {}
```

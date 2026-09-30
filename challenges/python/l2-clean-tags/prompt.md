Writers on a blog type their own tags, and they're messy. Write `clean_tags(tags)` that returns a new list where each tag has:

- the spaces at its start and end removed, and
- been made all lowercase.

Tags that are **empty after stripping** are dropped. Everything else stays, **in its original order** (duplicates included).

- `clean_tags(["  Python", "BEGINNER ", ""])` → `["python", "beginner"]`
- `clean_tags(["Go", "go"])` → `["go", "go"]`
- `clean_tags(["   "])` → `[]`

Try to write the whole function body as a single **list comprehension**.

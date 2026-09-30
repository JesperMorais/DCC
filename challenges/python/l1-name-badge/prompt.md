A conference prints name badges. People type their names into a form, often with stray spaces and random capitals, so the badge printer needs a clean version.

Write `name_badge(first, last)` that returns:

- the **first name in UPPERCASE**, then a space, then the **first letter of the last name** in uppercase, followed by a dot
- remove any spaces at the start or end of both names first
- if the last name is empty (or only spaces), return just the uppercase first name

Examples:

- `name_badge("ada", "lovelace")` → `"ADA L."`
- `name_badge("  grace ", " hopper")` → `"GRACE H."`
- `name_badge("Linus", "")` → `"LINUS"`

### Looping over a string

A `for` loop works on strings too. It hands you **one character at a time**, from left to right:

```python
for ch in "hi!":
    print(ch)    # "h", then "i", then "!"
```

### Asking a character questions

Strings have methods that answer yes/no questions about their characters:

```python
"7".isdigit()   # True   (0–9)
"a".isalpha()   # True   (a letter)
"Q".isupper()   # True
" ".isspace()   # True
```

### Counting (or collecting) as you go

Combine the two with a variable that you update inside the loop. Here we count the spaces in a sentence:

```python
sentence = "the quick brown fox"
spaces = 0
for ch in sentence:
    if ch == " ":
        spaces += 1     # short for: spaces = spaces + 1

spaces   # 3
```

`+=` also works on strings: `word += "s"` sticks an `"s"` onto the end of `word`.

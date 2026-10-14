### Where does an attribute live?

A Python object can find an attribute in two places: on the **instance** itself, or on its **class**. `obj.x` looks at the instance first and falls back to the class.

```python
class Dog:
    species = "Canis familiaris"     # class attribute: one value, shared

    def __init__(self, name: str) -> None:
        self.name = name             # instance attribute: one per object
```

Sharing is harmless for a string, because you never change it in place. Assigning `rex.species = "wolf"` creates a *new* instance attribute that hides the class one. But calling a mutating method like `.append()` doesn't assign anything. It changes whatever object the lookup found, wherever that object lives.

### Debugging technique: ask "is it the same object?"

When two things that should be independent influence each other, check identity rather than equality:

```python
a, b = Dog("Rex"), Dog("Fido")
print(a.name is b.name)   # False: two separate strings
print(vars(a))            # {'name': 'Rex'}: only what lives on the instance
```

`vars(obj)` shows what the instance itself holds. If something you expected per-object isn't in there, it lives somewhere shared.

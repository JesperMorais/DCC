Every call to the geocoding API costs money, and users type the same addresses over and over, with random capitalisation and stray spaces. Write **`make_geocoder`**, which wraps a `fetch` function with a small, bounded cache.

```python
type Coords = tuple[float, float]

def make_geocoder(fetch: Callable[[str], Coords], maxsize: int) -> Callable[[str], Coords]
```

The returned `lookup(address)`:

- **Normalises** the address with `.strip().casefold()` and calls `fetch` with the *normalised* string.
- Calls `fetch` at most once per normalised address, as long as the result is still cached.
- Keeps at most `maxsize` addresses. When full, it forgets the **least recently used** one (a cache hit counts as a use).
- Doesn't cache failures: if `fetch` raises, the exception propagates and the next lookup tries again.
- Each geocoder has its own cache.

```python
geo = make_geocoder(api.fetch, maxsize=100)
geo("Drottninggatan 1")      # calls api.fetch("drottninggatan 1")
geo("  DROTTNINGGATAN 1 ")   # cached, no API call
```

The standard library can do the hard part for you.

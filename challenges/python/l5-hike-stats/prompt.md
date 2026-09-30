Your GPS watch records the elevation (in metres) every minute of a hike. Write **`hike_stats`**, which summarises the track:

```python
def hike_stats(elevations: list[int]) -> tuple[int, int, int]
```

It returns `(total_ascent, total_descent, longest_climb)`:

- **total_ascent**: the sum of every uphill step between consecutive samples.
- **total_descent**: the sum of every downhill step, as a **positive** number.
- **longest_climb**: the biggest total gain of an uninterrupted climb. A climb is a run of consecutive **strictly** uphill steps, so a flat or downhill step ends it. "Biggest" means the most metres gained, not the most steps.
- Fewer than two samples means no steps: `(0, 0, 0)`.

```python
hike_stats([100, 120, 150, 140, 140, 160, 170, 175, 90])
# steps: +20 +30 -10 0 +20 +10 +5 -85
# (85, 95, 50): climbs gained 50 and 35

hike_stats([0, 10, 10, 20])   # (20, 0, 10): the flat step splits the climb
```

Try to write it with `itertools` rather than index arithmetic.

**Bug report from the greenhouse team:** "The daily summary always shows a low temperature, but the high is blank, every single day. The sensors are fine: the raw readings are all there."

`reading_stats(readings)` summarises one day of temperature readings. A reading is `None` when the sensor didn't answer. It returns a `ReadingStats` with:

- `count`: the number of real readings, and `missing`: the number of `None`s,
- `low` and `high`: the lowest and highest real reading, or `None` if there are none.

```python
reading_stats([None, None])
# ReadingStats(count=0, missing=2, low=None, high=None)

reading_stats([None, 12.0, None, 9.5])
# ReadingStats(count=2, missing=2, low=9.5, high=12.0)
```

**Reproduction:**

```python
reading_stats([4.5, -2.0, 7.25, 3.0])
# returns  ReadingStats(count=4, missing=0, low=-2.0, high=None)
# expected ReadingStats(count=4, missing=0, low=-2.0, high=7.25)
```

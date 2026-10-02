Your photo-sharing site makes thumbnails. Sizes arrive as text like `"4000x3000"` and are passed around as a **tuple** of width and height.

**1. Declare `type Size`** as a tuple of exactly two numbers: width, then height.

**2. `parseSize(text)`** turns `"<width>x<height>"` into a `Size`, or returns `null` if the text isn't a valid size.

- Spaces around the numbers are fine: `" 800 x 600 "` is `[800, 600]`.
- Both numbers must be whole numbers above zero. `"0x10"`, `"12.5x3"`, `"x600"` and `"abcx600"` are all `null`.
- There must be exactly one `x`: `"1x2x3"` is `null`.
- Its return type is `Size | null`.

**3. `fitInside(size, box)`** returns the size scaled so it fits inside `box`, keeping its aspect ratio.

- Scale **down** only. An image that already fits comes back unchanged.
- Round each side to the nearest whole pixel.
- Its return type is `Size`.

```ts
parseSize("1920x1080");             // [1920, 1080]
parseSize("1x2x3");                 // null
fitInside([4000, 3000], [800, 800]); // [800, 600]
fitInside([1000, 2000], [500, 500]); // [250, 500]
fitInside([300, 200], [800, 800]);   // [300, 200]
```

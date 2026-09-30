A music app has a "play backwards" button. The playlist is an array of song IDs. Write `reverse_into`, which writes the songs in **reverse order** into a second array:

```c
#include <stddef.h>

void reverse_into(const int *src, size_t count, int *out);
```

- `src` holds `count` song IDs. **Don't change it**, since the original playlist must stay as it was.
- `out` is an array the caller has already made, with room for exactly `count` ints. Fill it so that `out[0]` is the last song of `src`, and `out[count - 1]` is the first.
- If `count` is `0`, write nothing.

Example:

```c
int playlist[] = {101, 202, 303, 404, 505};
int backwards[5];
reverse_into(playlist, 5, backwards);
// backwards is now {505, 404, 303, 202, 101}
// playlist is still {101, 202, 303, 404, 505}
```

A music player keeps its playlist as a singly linked list. Add a "reverse order" button:

```c
typedef struct Song {
    int id;
    struct Song *next;
} Song;

Song *playlist_reverse(Song *head);
```

- Reverse the list **in place** by relinking the existing nodes, and return the new head.
- Don't allocate or free any nodes. The same nodes must come back in the opposite order: the old last node becomes the new head, and the old head's `next` becomes `NULL`.
- An empty list (`NULL`) stays `NULL`. A single song stays as it is.

```c
// 1 → 2 → 3 → NULL
Song *p = playlist_reverse(head);
// p: 3 → 2 → 1 → NULL, and p is the node that used to hold 3
```

The tests build the lists and free them afterwards, so you only relink.

### A linked list is nodes pointing at nodes

Each node holds a value and a pointer to the next node. The last node's `next` is `NULL`, and the whole list is represented by a pointer to its first node:

```c
typedef struct Item {
    int value;
    struct Item *next;   // the typedef name doesn't exist yet here, so use the struct tag
} Item;

for (const Item *it = first; it != NULL; it = it->next) {
    printf("%d\n", it->value);
}
```

Every node comes from its own `malloc`, so every node needs its own `free`.

### Why `Item **`?

C passes arguments by value, pointers included. If a function receives `Item *first` and assigns `first = something`, only its local copy changes. To change the *caller's* pointer, you pass the pointer's address:

```c
void clear(Item **list) {
    /* ... free the nodes ... */
    *list = NULL;   // now the caller's variable is NULL too
}

clear(&my_list);
```

### Freeing a chain

Once a block is freed you may not touch it again, not even to read one field from it. Think about which information you still need from a node *before* you give it back.

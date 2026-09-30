#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STRMAP_BUCKETS 64

typedef struct StrMap StrMap;

typedef struct Entry {
    char *key;             // owned copy
    int value;
    struct Entry *next;
} Entry;

struct StrMap {
    Entry *buckets[STRMAP_BUCKETS];   // each is the head of a chain, or NULL
    size_t size;
};

// Provided: FNV-1a hash of `key`, reduced to a bucket index.
static size_t bucket_of(const char *key) {
    uint32_t h = 2166136261u;
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= *p;
        h *= 16777619u;
    }
    return h % STRMAP_BUCKETS;
}

// Provided: an empty map (calloc makes every bucket NULL and size 0).
StrMap *strmap_new(void) {
    return calloc(1, sizeof(StrMap));
}

// Provided.
size_t strmap_size(const StrMap *m) {
    return m->size;
}

bool strmap_put(StrMap *m, const char *key, int value) {
    (void)m;
    (void)key;
    (void)value;
    (void)bucket_of;
    return false;
}

bool strmap_get(const StrMap *m, const char *key, int *out) {
    (void)m;
    (void)key;
    (void)out;
    return false;
}

void strmap_free(StrMap *m) {
    (void)m;
}

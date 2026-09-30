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
    Entry *buckets[STRMAP_BUCKETS];
    size_t size;
};

static size_t bucket_of(const char *key) {
    uint32_t h = 2166136261u;   // FNV-1a
    for (const unsigned char *p = (const unsigned char *)key; *p != '\0'; p++) {
        h ^= *p;
        h *= 16777619u;
    }
    return h % STRMAP_BUCKETS;
}

static Entry *find(const StrMap *m, const char *key) {
    for (Entry *e = m->buckets[bucket_of(key)]; e != NULL; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            return e;
        }
    }
    return NULL;
}

StrMap *strmap_new(void) {
    return calloc(1, sizeof(StrMap));   // all buckets NULL, size 0
}

bool strmap_put(StrMap *m, const char *key, int value) {
    Entry *existing = find(m, key);
    if (existing != NULL) {
        existing->value = value;
        return true;
    }

    Entry *e = malloc(sizeof *e);
    if (e == NULL) {
        return false;
    }
    size_t len = strlen(key) + 1;
    e->key = malloc(len);
    if (e->key == NULL) {
        free(e);
        return false;
    }
    memcpy(e->key, key, len);
    e->value = value;

    size_t b = bucket_of(key);
    e->next = m->buckets[b];
    m->buckets[b] = e;
    m->size++;
    return true;
}

bool strmap_get(const StrMap *m, const char *key, int *out) {
    const Entry *e = find(m, key);
    if (e == NULL) {
        return false;
    }
    *out = e->value;
    return true;
}

size_t strmap_size(const StrMap *m) {
    return m->size;
}

void strmap_free(StrMap *m) {
    if (m == NULL) {
        return;
    }
    for (size_t b = 0; b < STRMAP_BUCKETS; b++) {
        Entry *e = m->buckets[b];
        while (e != NULL) {
            Entry *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(m);
}

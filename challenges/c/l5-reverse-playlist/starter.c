#include <stddef.h>

typedef struct Song {
    int id;
    struct Song *next;
} Song;

Song *playlist_reverse(Song *head) {
    return head;
}

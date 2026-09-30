#include <stddef.h>

typedef struct Song {
    int id;
    struct Song *next;
} Song;

Song *playlist_reverse(Song *head) {
    Song *prev = NULL;
    while (head != NULL) {
        Song *next = head->next;   // save the rest of the list
        head->next = prev;         // turn this link around
        prev = head;
        head = next;
    }
    return prev;
}

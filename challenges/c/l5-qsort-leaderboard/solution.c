#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[16];
    int score;
} Player;

static int compare_players(const void *pa, const void *pb) {
    const Player *a = pa, *b = pb;
    if (a->score != b->score) {
        return a->score > b->score ? -1 : 1;   // higher score first
    }
    return strcmp(a->name, b->name);
}

void sort_leaderboard(Player *players, size_t count) {
    qsort(players, count, sizeof *players, compare_players);
}

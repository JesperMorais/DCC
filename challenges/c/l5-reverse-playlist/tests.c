#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// Test helpers: build a playlist from ids, copy it out, free it.
static Song *make_playlist(const int *ids, size_t n) {
    Song *head = NULL;
    for (size_t i = n; i > 0; i--) {
        Song *s = malloc(sizeof *s);
        s->id = ids[i - 1];
        s->next = head;
        head = s;
    }
    return head;
}

static size_t copy_ids(const Song *head, int *out, size_t max) {
    size_t n = 0;
    for (; head != NULL && n < max; head = head->next) out[n++] = head->id;
    return n;
}

static void free_playlist(Song *head) {
    while (head != NULL) {
        Song *next = head->next;
        free(head);
        head = next;
    }
}

TEST(reverses_five_songs) {
    int ids[] = {1, 2, 3, 4, 5};
    Song *p = playlist_reverse(make_playlist(ids, 5));
    int got[6] = {0};
    size_t n = copy_ids(p, got, 6);
    free_playlist(p);
    int expected[] = {5, 4, 3, 2, 1};
    EXPECT_EQ(n, 5);
    EXPECT_INT_ARRAY_EQ(got, expected, 5);
}

TEST(relinks_the_same_nodes_in_place) {
    int ids[] = {10, 20, 30};
    Song *head = make_playlist(ids, 3);
    Song *old_head = head, *old_tail = head->next->next;
    Song *p = playlist_reverse(head);
    bool new_head_is_old_tail = p == old_tail;
    bool old_head_ends_the_list = old_head->next == NULL;
    free_playlist(p);
    EXPECT_TRUE(new_head_is_old_tail);
    EXPECT_TRUE(old_head_ends_the_list);
}

TEST(empty_playlist_stays_empty) {
    EXPECT_NULL(playlist_reverse(NULL));
}

TEST(single_song_is_unchanged) {
    int ids[] = {7};
    Song *head = make_playlist(ids, 1);
    Song *p = playlist_reverse(head);
    bool same = p == head && p->next == NULL && p->id == 7;
    free_playlist(p);
    EXPECT_TRUE(same);
}

TEST(two_songs_swap) {
    int ids[] = {1, 2};
    Song *p = playlist_reverse(make_playlist(ids, 2));
    int got[3] = {0};
    size_t n = copy_ids(p, got, 3);
    free_playlist(p);
    int expected[] = {2, 1};
    EXPECT_EQ(n, 2);
    EXPECT_INT_ARRAY_EQ(got, expected, 2);
}

TEST(reversing_twice_restores_a_long_playlist) {
    int ids[1000];
    for (int i = 0; i < 1000; i++) ids[i] = i;
    Song *p = playlist_reverse(playlist_reverse(make_playlist(ids, 1000)));
    static int got[1001];
    size_t n = copy_ids(p, got, 1001);
    free_playlist(p);
    EXPECT_EQ(n, 1000);
    EXPECT_INT_ARRAY_EQ(got, ids, 1000);
}

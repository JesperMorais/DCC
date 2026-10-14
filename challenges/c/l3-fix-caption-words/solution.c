#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

size_t caption_word_count(const char *text) {
    size_t words = 0;
    bool in_word = false;
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (isspace((unsigned char)text[i])) {
            in_word = false;
        } else if (!in_word) {
            words++;
            in_word = true;
        }
    }
    return words;
}

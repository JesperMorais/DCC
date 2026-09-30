#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

void capitalize_words(char *s) {
    bool at_word_start = true;
    for (size_t i = 0; s[i] != '\0'; i++) {
        unsigned char c = (unsigned char)s[i];
        if (isspace(c)) {
            at_word_start = true;
        } else {
            s[i] = (char)(at_word_start ? toupper(c) : tolower(c));
            at_word_start = false;
        }
    }
}

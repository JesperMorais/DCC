#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

bool initials(const char *name, char *out, size_t cap) {
    if (cap == 0) {
        return false;
    }
    size_t n = 0;
    bool at_word_start = true;
    for (size_t i = 0; name[i] != '\0'; i++) {
        if (name[i] == ' ') {
            at_word_start = true;
        } else if (at_word_start) {
            if (n + 1 >= cap) {
                return false;
            }
            out[n++] = (char)toupper((unsigned char)name[i]);
            at_word_start = false;
        }
    }
    out[n] = '\0';
    return true;
}

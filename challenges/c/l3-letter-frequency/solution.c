#include <ctype.h>
#include <stddef.h>

void letter_counts(const char *text, int counts[26]) {
    for (int k = 0; k < 26; k++) {
        counts[k] = 0;
    }
    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned char c = (unsigned char)text[i];
        if (isalpha(c)) {
            counts[tolower(c) - 'a']++;
        }
    }
}

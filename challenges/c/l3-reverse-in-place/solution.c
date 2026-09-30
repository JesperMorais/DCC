#include <stddef.h>
#include <string.h>

void reverse_in_place(char *s) {
    size_t len = strlen(s);
    if (len < 2) {
        return;
    }
    for (size_t i = 0, j = len - 1; i < j; i++, j--) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

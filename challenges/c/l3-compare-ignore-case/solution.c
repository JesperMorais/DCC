#include <ctype.h>
#include <stddef.h>

int compare_ignore_case(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] != '\0' && tolower((unsigned char)a[i]) == tolower((unsigned char)b[i])) {
        i++;
    }
    return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
}

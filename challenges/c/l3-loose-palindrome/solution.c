#include <ctype.h>
#include <stdbool.h>
#include <string.h>

bool is_loose_palindrome(const char *s) {
    size_t len = strlen(s);
    if (len == 0) {
        return true;
    }
    size_t i = 0;
    size_t j = len - 1;
    while (i < j) {
        while (i < j && !isalpha((unsigned char)s[i])) i++;
        while (i < j && !isalpha((unsigned char)s[j])) j--;
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)s[j])) {
            return false;
        }
        i++;
        j--;
    }
    return true;
}

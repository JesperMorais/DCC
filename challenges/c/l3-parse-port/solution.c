#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

bool parse_port(const char *text, unsigned *out) {
    if (text[0] == '\0') {
        return false;
    }
    unsigned value = 0;
    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned char c = (unsigned char)text[i];
        if (!isdigit(c)) {
            return false;
        }
        value = value * 10 + (unsigned)(c - '0');
        if (value > 65535) {
            return false;
        }
    }
    *out = value;
    return true;
}

#include <ctype.h>

static int hex_nibble(char c) {
    int u = toupper((unsigned char)c);
    if (u >= '0' && u <= '9') {
        return u - '0';
    }
    if (u >= 'A' && u <= 'F') {
        return u - 'A';
    }
    return -1;
}

int hex_pair_value(const char *pair) {
    int high = hex_nibble(pair[0]);
    int low = hex_nibble(pair[1]);
    if (high < 0 || low < 0) {
        return -1;
    }
    return high * 16 + low;
}

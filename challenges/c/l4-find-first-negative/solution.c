#include <stddef.h>

int *find_first_negative(int *balances, size_t count) {
    if (balances == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < count; i++) {
        if (balances[i] < 0) {
            return &balances[i];
        }
    }
    return NULL;
}

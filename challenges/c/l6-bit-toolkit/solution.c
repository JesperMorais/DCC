#include <stdbool.h>
#include <stdint.h>

bool bit_is_set(uint32_t x, unsigned n) {
    return (x & (UINT32_C(1) << n)) != 0;
}

unsigned count_set_bits(uint32_t x) {
    unsigned count = 0;
    while (x != 0) {
        x &= x - 1;   // clear the lowest set bit
        count++;
    }
    return count;
}

bool is_power_of_two(uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

uint32_t reverse_bits(uint32_t x) {
    uint32_t r = 0;
    for (int i = 0; i < 32; i++) {
        r = (r << 1) | (x & 1u);
        x >>= 1;
    }
    return r;
}

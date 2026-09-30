#include <stddef.h>

void transpose(size_t rows, size_t cols,
               const int in[rows][cols], int out[cols][rows]) {
    for (size_t r = 0; r < rows; r++) {
        for (size_t c = 0; c < cols; c++) {
            out[c][r] = in[r][c];
        }
    }
}

#include <stddef.h>

size_t longest_goal_streak(const int *steps, size_t count, int goal) {
    size_t longest = 0;
    size_t current = 0;
    for (size_t i = 0; i < count; i++) {
        if (steps[i] >= goal) {
            current++;
        } else {
            if (current > longest) {
                longest = current;
            }
            current = 0;
        }
    }
    return longest;
}

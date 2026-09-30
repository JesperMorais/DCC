#include <stdbool.h>

bool is_working_day(int day) {
    switch (day) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return true;
        default:
            return false;
    }
}

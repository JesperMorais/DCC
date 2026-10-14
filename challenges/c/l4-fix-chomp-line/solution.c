#include <ctype.h>
#include <stddef.h>
#include <string.h>

size_t chomp_line(char *line) {
    char *end = line + strlen(line);
    while (end > line && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return (size_t)(end - line);
}

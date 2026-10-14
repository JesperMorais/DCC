#include <stddef.h>

typedef struct {
    const char *name;
    int opcode;
} Command;

static const Command COMMANDS[] = {
    {"start", 1},
    {"stop", 2},
    {"status", 3},
    {"reset", 4},
};

int command_opcode(const char *name) {
    const Command *end = COMMANDS + sizeof COMMANDS / sizeof COMMANDS[0];
    for (const Command *c = COMMANDS; c != end; c++) {
        if (c->name == name) {
            return c->opcode;
        }
    }
    return -1;
}

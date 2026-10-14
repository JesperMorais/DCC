**Bug report from the bench-test rig**

> The rig's debug shell maps typed commands to opcodes with `command_opcode`. The unit tests for it are all green, but on the actual rig every command we type comes back as "unknown command". We copy each typed word into a `char` buffer before looking it up, if that matters.

`command_opcode` looks a command name up in the `COMMANDS` table and returns its opcode, or `-1` if the name isn't in the table. Matching is exact and case-sensitive.

| name | opcode |
|---|---|
| `start` | 1 |
| `stop` | 2 |
| `status` | 3 |
| `reset` | 4 |

```c
command_opcode("status");   // 3
command_opcode("STOP");     // -1: case matters
command_opcode("halt");     // -1: not in the table
```

**Reproduction**

```c
char typed[16];
strcpy(typed, "stop");
command_opcode(typed);   // returns -1, expected 2
```

**This tree assumes you can already write C.** From the first node on, the lessons use structs, pointers, pointer casts and `uint32_t` without explaining them. The first project also needs strings, tokenising, function-pointer tables and careful number parsing. If any of that feels shaky, do the C track first. It's the same app: switch to **C** in the sidebar. These dailies cover exactly what this tree takes for granted:

| You'll need | Practise it with |
|---|---|
| Strings and buffers | [Where does the text end?](/solve/c/l3-my-strlen) · [Initials that fit](/solve/c/l3-initials-buffer) · [Parse a port number](/solve/c/l3-parse-port) |
| Pointers | [Point at the problem](/solve/c/l4-find-first-negative) · [Two answers, one function](/solve/c/l4-min-max-out-params) · [A cursor that moves itself](/solve/c/l4-next-word-cursor) |
| Structs | [Where windows overlap](/solve/c/l5-rect-overlap) · [Sort the leaderboard](/solve/c/l5-qsort-leaderboard) |
| Tokenising a command line | [Split a command line](/solve/c/l5-split-tokens) |
| Function pointers | [Pass the behaviour in](/solve/c/l6-map-and-fold) |
| Bits and masks | [A register bit toolkit](/solve/c/l6-bit-toolkit) · [Read, write, execute](/solve/c/l6-permission-flags) |
| Parsing numbers safely | [Parse numbers out of G-code](/solve/c/l6-parse-int-prefix) |

**Quick self-check:** can you explain what `uint32_t *p = (uint32_t *)0x40020014; *p |= 1u << 5;` does, write a function that splits `"led on 3"` into words in place, and call a function through a pointer stored in a struct? If yes, start with *Bits & registers*. If not, a week of C dailies first will make this tree far more fun.

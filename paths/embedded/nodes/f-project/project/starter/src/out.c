#include <stdarg.h>
#include <stdio.h>

#include "hal.h"
#include "out.h"

void out_putc(char c) {
    hal_putc(c);
}

void out_puts(const char *s) {
    while (*s) hal_putc(*s++);
}

void out_printf(const char *fmt, ...) {
    char buf[128];   /* on the stack, not the heap */
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    out_puts(buf);
}

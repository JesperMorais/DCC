/* Text output over the UART (given). */
#ifndef OUT_H
#define OUT_H

void out_putc(char c);
void out_puts(const char *s);   /* sends s as is: no newline is added */
void out_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));  /* up to 127 characters */

#endif

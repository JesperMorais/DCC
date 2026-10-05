#include <stdint.h>
#include <string.h>

#include "commands.h"
#include "hal.h"
#include "out.h"
#include "uart.h"

const shell_cmd_t shell_commands[] = {
    { "help",  cmd_help,  "list the commands" },
    { "echo",  cmd_echo,  "print the words back" },
    { "stats", cmd_stats, "show the UART error counters" },
    { "led",   cmd_led,   "on | off | blink <0-15>, or show the state" },
    { "gpio",  cmd_gpio,  "set | clear | get <pin 0-15>" },
    { "reg",   cmd_reg,   "read <offset> | write <offset> <value>" },
};
const size_t shell_num_commands = sizeof shell_commands / sizeof shell_commands[0];

static int ok(void) {
    out_puts("ok\r\n");
    return 0;
}

static int fail(const char *msg) {
    out_puts(msg);
    return -1;
}

/* ---- Milestone 3 ---- */

int cmd_help(int argc, char *argv[]) {
    (void)argc, (void)argv;
    for (size_t i = 0; i < shell_num_commands; i++)
        out_printf("%-8s%s\r\n", shell_commands[i].name, shell_commands[i].help);
    return 0;
}

int cmd_echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) out_puts(" ");
        out_puts(argv[i]);
    }
    out_puts("\r\n");
    return 0;
}

int cmd_stats(int argc, char *argv[]) {
    (void)argc, (void)argv;
    out_printf("rx dropped %lu, overruns %lu\r\n", (unsigned long)uart_rx_dropped(), (unsigned long)uart_overruns());
    return 0;
}

/* ---- Milestone 4 ---- */

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool parse_u32(const char *s, uint32_t *out) {
    uint32_t v = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        if (*s == '\0') return false;
        for (; *s; s++) {
            int d = hex_digit(*s);
            if (d < 0 || v > 0x0FFFFFFFu) return false;   /* another digit would push bits out the top */
            v = (v << 4) | (uint32_t)d;
        }
    } else {
        if (*s == '\0') return false;
        for (; *s; s++) {
            if (*s < '0' || *s > '9') return false;
            uint32_t d = (uint32_t)(*s - '0');
            if (v > (UINT32_MAX - d) / 10) return false;
            v = v * 10 + d;
        }
    }
    *out = v;
    return true;
}

int cmd_led(int argc, char *argv[]) {
    dev_regs_t *dev = hal_dev();
    if (argc == 1) {
        uint32_t ctrl = dev->CTRL;   /* one read, two fields */
        out_printf("led %s, blink %lu\r\n", (ctrl & DEV_CTRL_LED) ? "on" : "off",
                   (unsigned long)((ctrl & DEV_CTRL_BLINK_Msk) >> DEV_CTRL_BLINK_Pos));
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "on") == 0) {
        dev->CTRL |= DEV_CTRL_LED;
        return ok();
    }
    if (argc == 2 && strcmp(argv[1], "off") == 0) {
        dev->CTRL &= ~DEV_CTRL_LED;
        return ok();
    }
    if (argc == 3 && strcmp(argv[1], "blink") == 0) {
        uint32_t rate;
        if (!parse_u32(argv[2], &rate) || rate > 15) return fail("error: blink rate must be 0-15\r\n");
        dev->CTRL = (dev->CTRL & ~DEV_CTRL_BLINK_Msk) | ((rate << DEV_CTRL_BLINK_Pos) & DEV_CTRL_BLINK_Msk);
        return ok();
    }
    return fail("usage: led [on|off|blink <0-15>]\r\n");
}

int cmd_gpio(int argc, char *argv[]) {
    if (argc != 3 || (strcmp(argv[1], "set") != 0 && strcmp(argv[1], "clear") != 0 && strcmp(argv[1], "get") != 0))
        return fail("usage: gpio set|clear|get <pin>\r\n");
    uint32_t pin;
    if (!parse_u32(argv[2], &pin) || pin >= DEV_GPIO_PINS) return fail("error: pin must be 0-15\r\n");

    dev_regs_t *dev = hal_dev();
    if (strcmp(argv[1], "get") == 0) {
        out_printf("gpio %lu = %lu\r\n", (unsigned long)pin, (unsigned long)((dev->GPIO_IN >> pin) & 1u));
        return 0;
    }
    if (strcmp(argv[1], "set") == 0) dev->GPIO_OUT |= 1u << pin;
    else dev->GPIO_OUT &= ~(1u << pin);
    return ok();
}

/* The register at a byte offset into the device block. The offset is added to
 * the address as a number: adding it to a uint32_t pointer would scale it by 4. */
static volatile uint32_t *reg_at(uint32_t offset) {
    return (volatile uint32_t *)((uintptr_t)hal_dev() + offset);
}

int cmd_reg(int argc, char *argv[]) {
    int reading = argc == 3 && strcmp(argv[1], "read") == 0;
    int writing = argc == 4 && strcmp(argv[1], "write") == 0;
    if (!reading && !writing) return fail("usage: reg read <offset> | reg write <offset> <value>\r\n");

    uint32_t off;
    if (!parse_u32(argv[2], &off) || off % 4 != 0 || off >= sizeof(dev_regs_t)) {
        out_printf("error: offset must be 0x00-0x%02lX in steps of 4\r\n", (unsigned long)(sizeof(dev_regs_t) - 4));
        return -1;
    }
    if (reading) {
        out_printf("0x%02lX = 0x%08lX\r\n", (unsigned long)off, (unsigned long)*reg_at(off));
        return 0;
    }
    uint32_t value;
    if (!parse_u32(argv[3], &value)) return fail("error: bad value\r\n");
    if (off == offsetof(dev_regs_t, GPIO_IN) || off == offsetof(dev_regs_t, ID)) {
        out_printf("error: register 0x%02lX is read-only\r\n", (unsigned long)off);
        return -1;
    }
    *reg_at(off) = value;
    return ok();
}

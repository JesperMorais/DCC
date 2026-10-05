/* Milestone 4: numbers, and commands that drive the device registers. */
#include "check.h"
#include "commands.h"
#include "hal.h"
#include "shell.h"
#include "sim.h"

static void parse_good_numbers(void) {
    uint32_t v = 1234;
    CHECK(parse_u32("0", &v));
    CHECK_INT(v, 0);
    CHECK(parse_u32("42", &v));
    CHECK_INT(v, 42);
    CHECK(parse_u32("007", &v));
    CHECK_INT(v, 7);
    CHECK(parse_u32("0x2A", &v));
    CHECK_INT(v, 42);
    CHECK(parse_u32("0X2a", &v));
    CHECK_INT(v, 42);
    CHECK(parse_u32("4294967295", &v));
    CHECK_HEX(v, 0xFFFFFFFFu);
    CHECK(parse_u32("0xc0de0042", &v));
    CHECK_HEX(v, 0xC0DE0042u);
}

static void parse_bad_numbers(void) {
    const char *bad[] = { "", "0x", "-1", "+1", "12abc", " 5", "5 ", "4294967296",
                          "99999999999", "0x100000000", "0xG", "x10" };
    for (size_t i = 0; i < sizeof bad / sizeof bad[0]; i++) {
        uint32_t v = 1234;
        if (parse_u32(bad[i], &v)) check_fail(__FILE__, __LINE__, "parse_u32(\"%s\") should fail", bad[i]);
        if (v != 1234) check_fail(__FILE__, __LINE__, "parse_u32(\"%s\") changed *out on failure", bad[i]);
    }
}

static void led_on_off_keeps_other_bits(void) {
    dev_regs_t *dev = hal_dev();
    sim_reset();
    dev->CTRL = 0xA5A50030u;
    CHECK_INT(shell_exec("led on"), 0);
    CHECK_HEX(dev->CTRL, 0xA5A50031u);
    CHECK_STR(sim_tx(), "ok\r\n");
    CHECK_INT(shell_exec("led off"), 0);
    CHECK_HEX(dev->CTRL, 0xA5A50030u);
}

static void led_blink_sets_the_field(void) {
    dev_regs_t *dev = hal_dev();
    sim_reset();
    dev->CTRL = 0xA5A50001u;
    CHECK_INT(shell_exec("led blink 9"), 0);
    CHECK_HEX(dev->CTRL, 0xA5A50091u);
    CHECK_INT(shell_exec("led blink 15"), 0);
    CHECK_HEX(dev->CTRL, 0xA5A500F1u);
    CHECK_INT(shell_exec("led blink 0x2"), 0);
    CHECK_HEX(dev->CTRL, 0xA5A50021u);
}

static void led_blink_rejects_bad_rates(void) {
    dev_regs_t *dev = hal_dev();
    sim_reset();
    dev->CTRL = 0x31u;
    CHECK(shell_exec("led blink 16") != 0);
    CHECK_STR(sim_tx(), "error: blink rate must be 0-15\r\n");
    sim_tx_clear();
    CHECK(shell_exec("led blink fast") != 0);
    CHECK_STR(sim_tx(), "error: blink rate must be 0-15\r\n");
    CHECK_HEX(dev->CTRL, 0x31u);
}

static void led_shows_status(void) {
    sim_reset();
    hal_dev()->CTRL = 0xFF000051u;
    CHECK_INT(shell_exec("led"), 0);
    CHECK_STR(sim_tx(), "led on, blink 5\r\n");
    sim_tx_clear();
    hal_dev()->CTRL = 0x00000F00u;
    shell_exec("led");
    CHECK_STR(sim_tx(), "led off, blink 0\r\n");
}

static void led_usage(void) {
    sim_reset();
    CHECK(shell_exec("led dim") != 0);
    CHECK_STR(sim_tx(), "usage: led [on|off|blink <0-15>]\r\n");
    sim_tx_clear();
    CHECK(shell_exec("led on now") != 0);
    CHECK_PREFIX(sim_tx(), "usage: led");
    CHECK_HEX(hal_dev()->CTRL, 0);
}

static void gpio_set_and_clear(void) {
    dev_regs_t *dev = hal_dev();
    sim_reset();
    dev->GPIO_OUT = 0x8001u;
    CHECK_INT(shell_exec("gpio set 5"), 0);
    CHECK_HEX(dev->GPIO_OUT, 0x8021u);
    CHECK_STR(sim_tx(), "ok\r\n");
    CHECK_INT(shell_exec("gpio clear 15"), 0);
    CHECK_HEX(dev->GPIO_OUT, 0x0021u);
    CHECK_INT(shell_exec("gpio clear 15"), 0);
    CHECK_HEX(dev->GPIO_OUT, 0x0021u);
}

static void gpio_get_reads_inputs(void) {
    sim_reset();
    sim_gpio_drive(1u << 7);
    hal_dev()->GPIO_OUT = 1u << 6;   /* get reads what comes in, not what we drive */
    CHECK_INT(shell_exec("gpio get 7"), 0);
    CHECK_STR(sim_tx(), "gpio 7 = 1\r\n");
    sim_tx_clear();
    CHECK_INT(shell_exec("gpio get 6"), 0);
    CHECK_STR(sim_tx(), "gpio 6 = 0\r\n");
}

static void gpio_rejects_bad_input(void) {
    const char *bad_pin[] = { "gpio set 16", "gpio set -1", "gpio get x", "gpio clear 0x10" };
    sim_reset();
    for (size_t i = 0; i < 4; i++) {
        sim_tx_clear();
        if (shell_exec(bad_pin[i]) == 0) check_fail(__FILE__, __LINE__, "\"%s\" should fail", bad_pin[i]);
        CHECK_STR(sim_tx(), "error: pin must be 0-15\r\n");
    }
    sim_tx_clear();
    CHECK(shell_exec("gpio toggle 3") != 0);
    CHECK_STR(sim_tx(), "usage: gpio set|clear|get <pin>\r\n");
    sim_tx_clear();
    CHECK(shell_exec("gpio set") != 0);
    CHECK_PREFIX(sim_tx(), "usage: gpio");
    CHECK_HEX(hal_dev()->GPIO_OUT, 0);
}

static void reg_read(void) {
    sim_reset();
    hal_dev()->GPIO_OUT = 0x21u;
    CHECK_INT(shell_exec("reg read 0x0C"), 0);
    CHECK_STR(sim_tx(), "0x0C = 0xC0DE0042\r\n");
    sim_tx_clear();
    CHECK_INT(shell_exec("reg read 4"), 0);
    CHECK_STR(sim_tx(), "0x04 = 0x00000021\r\n");
    sim_tx_clear();
    sim_gpio_drive(0xBEEFu);
    CHECK_INT(shell_exec("reg read 8"), 0);
    CHECK_STR(sim_tx(), "0x08 = 0x0000BEEF\r\n");
}

static void reg_write(void) {
    sim_reset();
    CHECK_INT(shell_exec("reg write 0x04 0xBEEF"), 0);
    CHECK_HEX(hal_dev()->GPIO_OUT, 0xBEEFu);
    CHECK_STR(sim_tx(), "ok\r\n");
    CHECK_INT(shell_exec("reg write 0 0x31"), 0);
    CHECK_HEX(hal_dev()->CTRL, 0x31u);
    sim_tx_clear();
    shell_exec("led");
    CHECK_STR(sim_tx(), "led on, blink 3\r\n");
}

static void reg_rejects_bad_offsets(void) {
    const char *bad[] = { "reg read 0x02", "reg read 0x10", "reg read 64", "reg read abc", "reg write 0x10 1" };
    sim_reset();
    for (size_t i = 0; i < 5; i++) {
        sim_tx_clear();
        if (shell_exec(bad[i]) == 0) check_fail(__FILE__, __LINE__, "\"%s\" should fail", bad[i]);
        CHECK_STR(sim_tx(), "error: offset must be 0x00-0x0C in steps of 4\r\n");
    }
}

static void reg_respects_read_only(void) {
    sim_reset();
    sim_gpio_drive(0x5u);
    CHECK(shell_exec("reg write 0x0C 0") != 0);
    CHECK_STR(sim_tx(), "error: register 0x0C is read-only\r\n");
    sim_tx_clear();
    CHECK(shell_exec("reg write 8 0xFFFF") != 0);
    CHECK_STR(sim_tx(), "error: register 0x08 is read-only\r\n");
    CHECK_HEX(hal_dev()->ID, DEV_ID_VALUE);
    CHECK_HEX(hal_dev()->GPIO_IN, 0x5u);
}

static void reg_rejects_bad_values_and_forms(void) {
    sim_reset();
    CHECK(shell_exec("reg write 0x04 lots") != 0);
    CHECK_STR(sim_tx(), "error: bad value\r\n");
    sim_tx_clear();
    CHECK(shell_exec("reg peek 4") != 0);
    CHECK_STR(sim_tx(), "usage: reg read <offset> | reg write <offset> <value>\r\n");
    sim_tx_clear();
    CHECK(shell_exec("reg write 4") != 0);
    CHECK_PREFIX(sim_tx(), "usage: reg");
    CHECK_HEX(hal_dev()->GPIO_OUT, 0);
}

static void whole_device_over_the_uart(void) {
    const char *in = "led on\rgpio set 3\r";
    sim_reset();
    shell_init();
    sim_tx_clear();
    while (*in) sim_uart_rx((uint8_t)*in++);
    shell_poll();
    CHECK(hal_dev()->CTRL & DEV_CTRL_LED);
    CHECK_HEX(hal_dev()->GPIO_OUT, 1u << 3);
    CHECK_STR(sim_tx(), "led on\r\nok\r\n> gpio set 3\r\nok\r\n> ");
}

void suite_m4(void) {
    RUN(parse_good_numbers, "m4: parse_u32 reads decimal and 0x hex");
    RUN(parse_bad_numbers, "m4: parse_u32 rejects anything else and leaves *out alone");
    RUN(led_on_off_keeps_other_bits, "m4: led on/off changes only the LED bit");
    RUN(led_blink_sets_the_field, "m4: led blink writes the BLINK field and nothing else");
    RUN(led_blink_rejects_bad_rates, "m4: led blink rejects rates outside 0-15");
    RUN(led_shows_status, "m4: led on its own shows the state");
    RUN(led_usage, "m4: a wrong led command prints its usage");
    RUN(gpio_set_and_clear, "m4: gpio set/clear change one output bit");
    RUN(gpio_get_reads_inputs, "m4: gpio get reads GPIO_IN");
    RUN(gpio_rejects_bad_input, "m4: gpio rejects bad pins and forms");
    RUN(reg_read, "m4: reg read prints a register");
    RUN(reg_write, "m4: reg write writes a register");
    RUN(reg_rejects_bad_offsets, "m4: reg rejects misaligned and out-of-range offsets");
    RUN(reg_respects_read_only, "m4: reg write refuses the read-only registers");
    RUN(reg_rejects_bad_values_and_forms, "m4: reg rejects bad values and forms");
    RUN(whole_device_over_the_uart, "m4: commands typed over the UART drive the device");
}

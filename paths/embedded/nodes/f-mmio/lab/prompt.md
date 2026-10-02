First power-on of a new board. It has a green LED on **PA5** and a user button on **PC13**. The button is **active-low**: a pull-up holds the pin at 1, and pressing the button pulls it to 0.

Use the simulated MCU in `mcu.h` (`GPIOA`, `GPIOC`, with `MODER`, `IDR`, `ODR` and `BSRR`) and write:

```c
volatile uint32_t *gpio_reg(uintptr_t port_base, uint32_t offset);
void board_init(void);
void led_on(void);
void led_off(void);
bool button_pressed(void);
```

- **`gpio_reg`**: the datasheet gives registers as *base address + byte offset* (MODER `0x00`, IDR `0x04`, ODR `0x08`, BSRR `0x0C`). Return a pointer to that register. For example, `gpio_reg((uintptr_t)GPIOA, 0x08)` must point at `GPIOA->ODR`.
- **`board_init`**: make PA5 an **output** (`01` in its 2-bit MODER field) and PC13 an **input** (`00`). Don't change the mode of any other pin. Port A's MODER comes out of reset as `0xABFFFFFF`, and PA13/PA14 are your debugger's SWD pins.
- **`led_on` / `led_off`**: drive PA5 **through BSRR only**, so an interrupt that changes other pins can never be lost. Write the set half (bits 0–15) to turn it on and the reset half (bits 16–31) to turn it off. Don't touch ODR yourself. In the tests, `sim_gpio_apply_bsrr(GPIOA)` plays the silicon that turns your BSRR write into an ODR change.
- **`button_pressed`**: return `true` while PC13 reads **low**. Ignore every other pin on port C.

`LED_PIN` (5) and `BUTTON_PIN` (13) are defined in the starter.

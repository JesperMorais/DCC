### A sensor node you can run on your laptop

Every Zephyr lab so far ran on a simulator built for the course. This time it's the real thing: the actual Zephyr kernel, built with `west`, configured with Kconfig and a devicetree, and running as an ordinary Linux program on the `native_sim` board. Nothing to flash and no SDK, but every API and every build step is exactly what you'd use on an nRF52 or an STM32.

You're building the firmware for a small **sensor node**: a box on a wall that measures the temperature, keeps statistics, reports them on a schedule and lets a technician poke at it over a serial console. Real products like this (building monitoring, cold-chain loggers, the condition monitors on pumps) have the same skeleton:

- a **driver** for the sensor, found through the **devicetree**, so the C never hard-codes which hardware it's talking to. There's no real sensor here, so you'll write the driver for a pretend one whose values follow a ramp, and that makes them easy to test;
- a **sampling thread** that hands readings to a **processing thread** through a `k_msgq`, and drops rather than blocks when it falls behind;
- statistics behind a `k_mutex`, a **`k_timer` that defers to a work item** for the periodic report, and **logging** through `LOG_MODULE_REGISTER`;
- a **shell command** for the operator, with input that's validated, not trusted.

That's the whole Zephyr branch in one image, plus the parts the labs couldn't give you: setting up a workspace, writing Kconfig options and a devicetree overlay, and getting the build to agree with you.

The milestones say what must work and how you'll know. The design is yours. The tests are a ztest suite that builds your code into its own image and talks to it through the standard sensor API and your shell command, so any sensible structure passes.

**How to start:** copy the starter (the command is on this page), open its `README.md` and do the one-time setup first (the Workshop lesson walks through it). Then `make run`, and on to milestone 1.

Setup can eat an evening, and the first devicetree error can eat another. That isn't you being slow; it's what Zephyr work actually looks like, and every error you read to the end is one you'll recognise for good. Take one milestone, then one test, at a time, commit each time it goes green, and take a hint the moment you're going in circles.

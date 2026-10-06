### A daemon that runs for a year

The greenhouse controller works on the bench. Now it has to live in a cabinet for a year with nobody watching. A humidity probe streams readings over a serial line, a temperature sensor sits behind an IIO driver, and someone wants a CSV of both, every second, forever. They also want to change the sampling rate without a site visit, never fill the 4 GB eMMC, and ask the box "are you OK?" over SSH. It has to survive `systemctl stop` at the worst possible moment.

You're going to build that program: **sensord**, a real userspace daemon in C on real Linux APIs. On your PC the hardware is faked with plain files. A **FIFO** stands in for the serial sensor, and a directory of text files stands in for the IIO channel in sysfs. Everything else is the real thing:

- one **epoll** loop that sleeps until something happens
- a **timerfd** that keeps a fixed sampling grid
- a **signalfd** for a clean shutdown (SIGTERM) and a config reload (SIGHUP)
- **rotated CSV logs**
- a **UNIX socket** you can ask for live counters

The README explains how each piece maps onto a real board: an IIO device, a UART, libgpiod and a systemd unit.

This is the Embedded Linux branch put together, with the least scaffolding of any project. You get user stories and the exact file and protocol formats: no headers and no stubs. The structure is yours (the README has an optional file layout and a `main.c` skeleton if you'd like somewhere to start). The tests are black-box. They start your binary in a temp directory, write to its FIFO, send it signals, talk to its socket and read its CSV files, so any sound design passes. They're also deliberately unkind about the production bugs from the boss: half lines, garbage, writers that come and go, signals at awkward moments, and fd leaks.

**How to start:** copy the starter (the command is on this page), run `git init` and `make test-m1` to watch everything fail, read `README.md`, and begin with milestone 1. Before you write the event loop, sketch on paper which file descriptors exist and who owns each one.

This is the biggest thing in the branch, and it's meant to take many sittings. A day spent staring at one `strace` until the EOF spin makes sense is a good day: that understanding is what the project is for, and it stays with you long after the tests are green. Take one milestone at a time, commit each time it goes green, and reach for a hint whenever you've been circling for a while.

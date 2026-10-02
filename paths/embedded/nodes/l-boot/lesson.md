You put the gear lever of a 2018 car into reverse. US law (FMVSS 111) says the rear-view camera image has to appear on the dash **within 2 seconds**. The infotainment head unit runs Linux, and it was asleep or switched off a moment ago. A stock desktop distro needs 20–40 s to boot. Somebody had to get that down to under 2 s, and to do that, they had to know every stage between "power good" and "camera app running".

This node walks through those stages. When a board stays dark, or hangs halfway through, the last line on the serial console tells you which stage died. The skill you're learning here is reading that line.

### The chain: each stage loads the next

On a microcontroller in Fundamentals, the reset vector jumped straight into your `main()`. Linux-class SoCs can't do that: DRAM isn't even running at reset. So booting is a **relay race**, where each runner is smarter than the last.

```
power on
  │
  ▼
1. Boot ROM      (in silicon, can't be changed) reads the boot pins/fuses,
                 finds the next stage on SD / eMMC / SPI NOR / USB, loads it into on-chip SRAM
  ▼
2. SPL / TPL     ("secondary program loader", a tiny U-Boot of ~100 KB)
                 initialises the DRAM controller and clocks, loads full U-Boot into DRAM
  ▼
3. U-Boot        reads its environment, runs `bootcmd`: loads the kernel, DTB and initramfs
                 (often from one FIT image), passes `bootargs`, jumps to the kernel
  ▼
4. Linux kernel  decompresses, parses the device tree, probes drivers, mounts root
  ▼
5. initramfs     (optional) a small RAM filesystem: unlock/verify/find the real rootfs, switch_root
  ▼
6. init          PID 1, usually systemd (or BusyBox init): starts your services
```

On many ARM64 parts, **TF-A** (Trusted Firmware-A, the "BL31" secure monitor) and sometimes OP-TEE run between SPL and U-Boot. They stay resident in secure memory after Linux starts.

### U-Boot: environment, bootcmd and FIT

U-Boot is a small OS in its own right, with a shell, drivers, a filesystem and a network stack. What it does on boot is driven by **environment variables**, stored in flash or eMMC:

```
=> printenv bootcmd bootargs
bootcmd=load mmc 0:1 ${loadaddr} /boot/image.itb; bootm ${loadaddr}#conf-board-revb
bootargs=console=ttymxc0,115200 root=/dev/mmcblk0p2 rootwait quiet
=> setenv bootdelay 0     # don't wait 3 s for a keypress
=> saveenv
```

After `bootdelay` runs out, U-Boot executes `bootcmd`. `bootargs` becomes the kernel command line.

A **FIT image** (Flattened Image Tree) packs the kernel, one or more DTBs and the initramfs into a single file. It also holds hashes and, optionally, signatures. It's described by an `.its` source file, which uses device-tree syntax:

```
images {
    kernel { data = /incbin/("Image.gz"); type = "kernel"; arch = "arm64";
             compression = "gzip"; load = <0x40480000>; entry = <0x40480000>;
             hash { algo = "sha256"; }; };
    fdt-revb { data = /incbin/("board-revb.dtb"); type = "flat_dt"; hash { algo = "sha256"; }; };
};
configurations {
    default = "conf-board-revb";
    conf-board-revb { kernel = "kernel"; fdt = "fdt-revb";
                      signature { algo = "sha256,rsa2048"; key-name-hint = "prod"; sign-images = "kernel", "fdt"; }; };
};
```

One file means one hash check. It also means you can't accidentally combine kernel v2 with the DTB from v1.

### Worked example: reading a boot log

```
U-Boot SPL 2024.01 (Mar 03 2025)            ← stage 2 ran: the ROM found it
DRAM:  1 GiB                                ← DRAM training worked
U-Boot 2024.01 ...                          ← stage 3
Hit any key to stop autoboot:  0
## Loading kernel from FIT Image at 40480000 ...
   Verifying Hash Integrity ... sha256+ OK
## Flattened Device Tree blob at 43000000
Starting kernel ...                         ← U-Boot's last words
[    0.000000] Booting Linux on physical CPU 0x0
...
[    1.913201] Run /init as init process     ← initramfs/rootfs reached
```

Each line moves the blame one stage further. If there's no output at all, suspect the boot pins, the ROM, or SPL on the wrong media or at the wrong offset. If it prints `DRAM:` and then dies, look at DRAM timing. The most famous hang is **`Starting kernel ...` followed by silence**. U-Boot did its job, so one of these is the usual culprit:
- **The kernel is printing somewhere you're not looking.** `console=` in bootargs, or `stdout-path` in the DTB, names the wrong UART, or the serial driver isn't built in. Add `earlycon` (with the right UART address) to see the earliest messages.
- **Wrong DTB.** It's for another board revision, or it doesn't match the kernel version, so the kernel can't find its memory, interrupt controller or console.
- **Bad load addresses.** The kernel, DTB and initramfs overlap in RAM, or the decompressed kernel overwrites the DTB.

### Secure boot in one paragraph

The Boot ROM holds the root of trust: a hash of a public key, burned into one-time-programmable **fuses**. The ROM verifies SPL's signature against it, SPL verifies U-Boot (or TF-A), U-Boot verifies the signed FIT configuration, and the kernel verifies the rootfs (with dm-verity) or modules. Every link checks the next one before running it, so this is a **chain of trust**. Blowing the "closed" fuse is irreversible, so practise on a sacrificial unit. And a U-Boot that lets anyone at the serial console type `setenv bootcmd` defeats everything: lock the environment and disable the interactive console in production.

### Gotchas

- **`saveenv` persists forever.** A typo in `bootcmd` survives reboots, and `env default -a` is how you get back. In production, many teams compile the environment in and make it read-only.
- **The initramfs isn't magic.** If the driver for `root=` is a module *inside the rootfs*, nothing can mount it. The initramfs solves that chicken-and-egg problem, or you build the driver in.
- **Boot time adds up.** Typical budget killers are a 3 s `bootdelay`, DRAM training, a gzip-compressed kernel on slow flash, probing every driver in the kernel, `udev` settling, and systemd starting 80 units you don't need. Measure with `grabserial`, `systemd-analyze blame` and `initcall_debug`.

### Mapping to Fundamentals

The MCU world has the same chain, just shorter. On an STM32, the ROM picks a boot source from the BOOT pins, and the reset handler sets up the clocks and `.data`/`.bss` (SPL's job, in 50 lines). `vTaskStartScheduler()` plays the part of `init`.

### In the wild

- **Rear-view cameras.** Head units hit the 2 s rule by cheating cleverly. Some have a small MCU or RTOS core on the same SoC that shows the camera image while Linux is still booting. Others use **Falcon mode**, where SPL boots the kernel directly and skips full U-Boot, with a trimmed, built-in-only kernel, and start the camera service before anything else in systemd. Some use suspend-to-RAM instead of a cold boot.
- **A/B updates** (Mender, RAUC, SWUpdate) use U-Boot's `bootcount` and `altbootcmd`. If the new image fails to boot 3 times, U-Boot falls back to the old slot automatically.
- **Interview insight:** "What happens between power-on and `main()` / PID 1?" is a classic question. A strong answer names each stage, **what initialises DRAM** (SPL), **how the kernel learns about the hardware** (the DTB passed in a register), and **how you'd debug a hang at each step** (the last line on the console, earlycon, JTAG).

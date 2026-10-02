Your first embedded Linux product shipped on a vendor's "SDK image": an Ubuntu-ish rootfs, 2 GB, with `apt` and Python 2 on it, plus a kernel that nobody could rebuild because the vendor's tarball had a missing patch. Eighteen months later a CVE lands in OpenSSL, and nobody can say exactly what's on the device.

That's why real products are **built from source, reproducibly**, by a build system that knows every package, version and patch. On a microcontroller you had one `make` and one ELF. Here you have a cross-compiler, a bootloader, a kernel, a device tree and hundreds of userspace packages, and something has to build all of them consistently.

### Cross-compiling, the toolchain and the sysroot

Your laptop is x86-64 and the board is ARM64. A **cross toolchain** is a compiler that runs on one and emits code for the other:

```
aarch64-linux-gnu-gcc -o sensord sensord.c        # host: x86-64, target: aarch64
file sensord   →  ELF 64-bit LSB executable, ARM aarch64
```

The compiler also needs the **target's** headers and libraries: its glibc, its `libgpiod.so`, its `openssl.h`. Those live in a **sysroot**, a directory that mirrors the target's `/usr`:

```
aarch64-linux-gnu-gcc --sysroot=$SDK/sysroots/cortexa53-poky-linux \
    sensord.c -lgpiod -o sensord
```

If you forget the sysroot, the compiler finds your **host's** `/usr/include` and `/usr/lib`. Then you get `skipping incompatible /usr/lib/x86_64-linux-gnu/libgpiod.so` or `file in wrong format` at link time, or, worse, code built against the wrong header versions. Rule of thumb: never let target builds see host paths. `pkg-config` needs `PKG_CONFIG_SYSROOT_DIR` for the same reason.

### Buildroot vs Yocto

Both download sources, build a toolchain, then build the bootloader, the kernel and the rootfs. Their philosophies differ:

| | **Buildroot** | **Yocto / OpenEmbedded** |
|---|---|---|
| Model | `make menuconfig` → one image | Layers of recipes → images, SDKs and a package feed |
| Learning curve | An afternoon | Weeks |
| First build | 20–40 min | 1–3 h (then sstate cache makes rebuilds fast) |
| Output | A firmware image | Images plus packages (ipk/rpm/deb), an SDK, licence manifests |
| Sweet spot | Small team, one product, fixed function | Many products/boards, a vendor BSP, long maintenance, compliance |

Choose **Buildroot** when the rootfs is small and the device just needs to do one job: you'll understand the whole thing. Choose **Yocto** when you have several boards, a silicon vendor that ships its BSP as a Yocto layer (NXP, TI, ST, NVIDIA and Xilinx all do), or a product line you'll maintain for ten years.

### Yocto vocabulary in five minutes

- **Recipe (`.bb`):** how to fetch, patch, configure, compile and install one piece of software, such as `sensord_1.2.bb`.
- **`.bbappend`:** a modification to *someone else's* recipe, kept in *your* layer: extra patches, config fragments, a different `defconfig`.
- **Layer (`meta-*`):** a git repo of recipes and config. Examples are `meta-openembedded`, the vendor's `meta-imx` (the **BSP layer**), and your own `meta-acme`.
- **`MACHINE`:** which board you're building for, selecting the kernel, DTBs and bootloader. **`DISTRO`:** policy (init system, libc, features).
- **bitbake:** the task engine. `bitbake acme-image` resolves the dependencies and runs `do_fetch → do_patch → do_compile → do_install → do_package…` for each recipe.

```
# meta-acme/recipes-kernel/linux/linux-imx_%.bbappend
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://0001-add-acme-board-dts.patch file://acme.cfg"
```

The golden rule: **never edit the vendor's layer or poky**. Put every change in your own layer as a `.bbappend`, so you can upgrade the vendor layer without losing your work.

### The device tree, Linux edition

You met devicetree in Zephyr. Linux invented it. A **DTB** describes hardware that can't be discovered by probing: which UART is at which address, what's on I²C bus 2, which GPIO resets the sensor. U-Boot passes it to the kernel at boot.

```dts
/* acme-board.dts */
#include "imx8mm.dtsi"            /* the SoC: shared by every board that uses this chip */

&i2c2 {
    status = "okay";
    temp@48 {
        compatible = "ti,tmp102";  /* ← the binding key */
        reg = <0x48>;
        interrupt-parent = <&gpio1>;
        interrupts = <9 IRQ_TYPE_LEVEL_LOW>;
    };
};
```

- **`.dtsi`** files describe the SoC (or a module), and the board's **`.dts`** includes them and enables or adds things with `&label { … }`.
- **`compatible`** is how a node finds its driver. The `tmp102` driver has an `of_match_table` containing `"ti,tmp102"`, and when the strings match, the kernel calls the driver's `probe()`. If they don't match, **nothing happens, silently**: no error, no device in `/sys/bus/i2c/devices/…/driver`.
- **Overlays (`.dtbo`)** patch the tree at boot time without rebuilding the base DTB. They're perfect for add-on boards (Raspberry Pi HATs, BeagleBone capes) or for variants detected by U-Boot.

### Worked example: "my sensor driver doesn't load"

1. Is the node in the **running** tree? `ls /proc/device-tree/soc/…/i2c@…/` or `dtc -I fs /sys/firmware/devicetree/base`. You may have edited a DTS that isn't the one U-Boot actually loads.
2. `status = "okay"`? A disabled parent bus (`&i2c2` left at `"disabled"`) hides every child.
3. Is the driver built (`CONFIG_SENSORS_TMP102=y/m`), and is the module loaded?
4. Does the `compatible` string match the driver's table **exactly**, vendor prefix included? `"tmp102"` and `"ti,tmp-102"` don't bind.
5. `dmesg | grep -i tmp` shows whether `probe()` ran and failed (for example `-EPROBE_DEFER`, waiting for a regulator), or never ran at all.

### Gotchas

- **The kernel and DTB must match.** Bindings change between kernel versions, so ship them together, ideally in one FIT.
- **Hand-built binaries "work on my board" until the next image.** Put them in a recipe, or they aren't part of the product.
- **Yocto's `DL_DIR` and `SSTATE_DIR`** should be shared and cached in CI, or every build takes hours.
- **Licences:** both tools generate a licence manifest. Your legal team will want it, especially for GPLv3 in the rootfs.

### In the wild

- **Fundamentals mapping:** the Fundamentals labs had one ELF and a linker script. Here the "linker script for the whole system" is the image recipe, and the device tree replaces the `#define UART_BASE 0x40011000` you'd hard-code in firmware, so the same kernel binary runs on fifty boards.
- **Automotive and industrial** (Automotive Grade Linux, Siemens' Industrial Linux, most NXP i.MX products) standardise on Yocto because of its layers and long-term maintenance. **Routers, 3D printers and small appliances** often use Buildroot or OpenWrt.
- **Interview insight:** "How would you add a driver change for your board without forking the vendor BSP?" The answer they want is: a `.bbappend` in your own layer adding the patch or kernel config fragment, plus a DTS (or overlay) with the right `compatible`. Bonus points for mentioning sysroots and why `pkg-config` must not see host libraries.

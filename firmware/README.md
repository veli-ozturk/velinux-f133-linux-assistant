# Firmware, flash images and vendor dumps

Shared to help others working on Allwinner F133 / D1s boards. **Use at your own risk** — writing the wrong image
can leave a board in FEL-only mode (it is recoverable, see below).

> ⚠️ Vendor binaries (`vendor/`, and the stock-derived parts of the flash images) belong to their respective
> owners and are **not** covered by this repository's MIT license. They are provided for research, interoperability
> and recovery. If you are a rights holder and want something removed, open an issue.

## Contents

| Folder | What | Origin |
|---|---|---|
| `flash-backup/flash-41-sirsiz-20261029.bin` | Full 16 MB SPI-NOR image of a working VeLinux board (kernel #41, package #46). The `/data` JFFS2 region `0xC00000–0xFE0000` is blanked to `0xFF` (no keys, tokens or Wi-Fi). | own build + stock boot0/UDISK |
| `flash-backup/sd_fel_switcher.bin` | **SD FEL switch**: 16 KB written to SD sectors 16..47 makes the board drop into FEL at power-on, even when the NOR boot is broken | own |
| `flash-backup/sd-ilk-1MB-*.bin` | First 1 MB of the SD card (MBR + gap) as used | own |
| `flash-package-46/` | Boot package #46 split into items: `item-melis.bin` = Linux 5.4 kernel, `item-rtos.bin` = initramfs (`F133INRD` header), `item-uboot.bin` = bridge + DTB, `item-opensbi.bin`, assembled `toc1.bin`, `board.dtb` | own build (item names kept from the stock TOC layout) |
| `kernel-v7-config/` | Kernel config and DTS/config diffs of an earlier (v7, sound + zram) package | own |
| `tina-opensbi/` | OpenSBI for sun20iw1p1 + entry/bridge stubs and DTB used to boot Linux from RAM over FEL | Tina SDK (OpenSBI: BSD-2) + own stubs |
| `vendor/ivt244-unpacked/` | Stock Melis firmware unpacked: boot package, `sys_config`, partition table, Melis package | device vendor |
| `vendor/minfs-decompressed/` | Stock Melis ROOTFS (MINFS) contents: `init.axf` (main app, with ELF copy), `desktop.mod`, `update.mod`, `Config.ini` | device vendor |
| `vendor/b5300-original/` | Stock config files and a normal boot log of the original firmware | device vendor |
| `vendor/tina-reference/` | Allwinner Tina SDK reference for F133: boot0/OpenSBI/U-Boot binaries, EVB board DTS, sys_config, kernel configs, partition configs | Allwinner Tina SDK |

Checksums: `SHA256SUMS` in `flash-backup/` and `flash-package-46/`.

## Flash map (16 MB SPI-NOR)

| Offset | Content |
|---|---|
| `0x000000` | boot0 (eGON.BT0) |
| `0x010000` | TOC1 boot package: OpenSBI, U-Boot bridge + DTB, Linux kernel, initramfs |
| `0xC00000` | `/data` JFFS2 (keys, Wi-Fi, settings — blanked in the backup) |
| `0xFE0000` | UDISK (small FAT, from stock firmware) |

## Recovery (board does not boot)

1. Write the SD FEL switch to an SD card from a PC: `python pc/sd_fel_anahtar_pc_v1.py <disk_no> yaz` (admin).
2. Insert the card, power-cycle → the board enters FEL (USB).
3. Write only the boot package: `python pc/fel_flash_linux_v7.py yaz --dizin firmware/flash-package-46`
   (backs up first, verifies by double read, writes the header sector last; does not touch `/data`).
4. Remove the switch (`... <disk_no> sil`), power-cycle.

Writing the full `flash-41` image with `xfel spinor write 0` also works but erases `/data` — last resort only.
If the board's UART MCU is awake it may reset the SoC ~10 s into FEL; keep the MCU idle first.

---

**Türkçe:** Bu klasörde VeLinux'un flash yedeği (sırları silinmiş), #46 açılış paketi, SD FEL anahtarı ve üretici
firmware dökümleri var. Üretici ikilileri sahiplerine aittir, MIT lisansına dahil değildir; araştırma ve kurtarma
amaçlı paylaşılmıştır. Kurtarma adımları yukarıda.

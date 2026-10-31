# Firmware, flash images and vendor dumps / Firmware, flash görüntüleri ve üretici dökümleri

**[English](#english)** · **[Türkçe](#türkçe)** · [← Main page / Ana sayfa](../README.md)

---

# English

Shared to help others working on Allwinner **F133 / D1s** boards. **Use at your own risk** — writing the wrong image
can leave a board in FEL-only mode. It is recoverable; see [Recovery](#recovery).

> ⚠️ Vendor binaries (`vendor/`, and the stock-derived parts of the flash images) belong to their respective owners
> and are **not** covered by this repository's MIT license. They are provided for research, interoperability and
> recovery. If you are a rights holder and want something removed, please open an issue.

## Contents

| Folder / file | What it is | Origin |
|---|---|---|
| `flash-backup/flash-41-sirsiz-20261029.bin` | Full **16 MB SPI-NOR image** of a working VeLinux board (kernel #41, boot package #46). The `/data` JFFS2 region `0xC00000–0xFE0000` is **blanked to 0xFF**: no keys, tokens or Wi-Fi settings. | own build + stock boot0 / UDISK |
| `flash-backup/sd_fel_switcher.bin` | **SD FEL switch**: 16 KB written to SD sectors 16..47. With this card inserted the board drops into USB FEL mode at power-on, even if the flash does not boot. | own |
| `flash-backup/sd-ilk-1MB-*.bin` | First 1 MB of the SD card (MBR + gap) as used on the device | own |
| `flash-package-46/` | Boot package #46 split into items: `item-melis.bin` = **Linux 5.4 kernel**, `item-rtos.bin` = **initramfs** (`F133INRD` header), `item-uboot.bin` = bridge + DTB, `item-opensbi.bin` = OpenSBI, `toc1.bin` = the assembled package, `board.dtb`. (Item names are kept from the stock TOC layout.) | own build |
| `kernel-v7-config/` | Kernel config plus DTS and config diffs of an earlier package (v7: sound + zram) | own |
| `tina-opensbi/` | OpenSBI for sun20iw1p1, entry/bridge stubs and the DTB used to boot Linux **from RAM over FEL** (for testing a kernel before writing flash) | Tina SDK (OpenSBI: BSD-2) + own stubs |
| `vendor/ivt244-unpacked/` | Stock Melis firmware, unpacked: U-Boot boot package, `sys_config`, partition table, Melis package | device vendor |
| `vendor/minfs-decompressed/` | Stock Melis root file system (MINFS) contents: `init.axf` (main application, also as ELF), `desktop.mod`, `update.mod`, `Config.ini` | device vendor |
| `vendor/b5300-original/` | Stock configuration files and a normal boot log of the original firmware | device vendor |
| `vendor/tina-reference/` | Allwinner **Tina SDK** reference for F133: boot0 / OpenSBI / U-Boot binaries, EVB board DTS, `sys_config`, kernel configs, partition and image configs, Wi-Fi platform code | Allwinner Tina SDK |

Checksums: `SHA256SUMS` in `flash-backup/` and `flash-package-46/` (`sha256sum -c SHA256SUMS`).

## Flash map (16 MB SPI-NOR)

| Offset | Size | Content |
|---|---|---|
| `0x000000` | 64 KB | boot0 (eGON.BT0) |
| `0x010000` | ~12 MB | TOC1 boot package: OpenSBI, U-Boot bridge + DTB, Linux kernel, initramfs |
| `0xC00000` | ~3.9 MB | `/data` (JFFS2): keys, Wi-Fi, settings — **blanked in the backup** |
| `0xFE0000` | 128 KB | UDISK (small FAT kept from the stock firmware) |

## Recovery

When the board does not boot any more:

1. **Make an SD FEL switch** on a PC (administrator rights):
   `python pc/sd_fel_anahtar_pc_v1.py <disk_no> yaz` — writes `sd_fel_switcher.bin` to sectors 16..47 of that disk.
2. **Enter FEL:** insert the card into the board and power-cycle. The board appears as a USB FEL device.
3. **Write only the boot package** (keeps `/data`):
   `python pc/fel_flash_linux_v7.py yaz --dizin firmware/flash-package-46`
   The tool backs up the flash first, verifies by double read and writes the header sector last.
4. **Remove the switch:** `python pc/sd_fel_anahtar_pc_v1.py <disk_no> sil`, put the card back, power-cycle.

Writing the whole `flash-41` image with `xfel spinor write 0 flash-41-sirsiz-20261029.bin` also works, but it erases
`/data` (keys and Wi-Fi must be written again) — use it only as a last resort.

**Tips**
- The board's Bluetooth/audio MCU resets the SoC about 10 s into FEL if it is awake and unanswered; keep it idle
  (stop the services that talk to it) before starting long FEL work.
- A kernel that boots from RAM over FEL may still fail from flash. Always have the SD FEL switch ready before writing.

---

# Türkçe

Allwinner **F133 / D1s** kartlarıyla uğraşanlara yardımcı olsun diye paylaşıldı. **Sorumluluk kullanıcıya aittir** —
yanlış görüntü yazmak kartı yalnız FEL kipinde açılır hâle getirebilir. Kurtarılabilir; bkz. [Kurtarma](#kurtarma).

> ⚠️ Üretici ikilileri (`vendor/` ve flash görüntülerinin özgün firmware'den gelen kısımları) sahiplerine aittir ve
> bu deponun MIT lisansına **dahil değildir**. Araştırma, birlikte çalışabilirlik ve kurtarma amacıyla paylaşılmıştır.
> Hak sahibiyseniz ve bir şeyin kaldırılmasını istiyorsanız lütfen bir issue açın.

## İçindekiler

| Klasör / dosya | Ne | Kaynak |
|---|---|---|
| `flash-backup/flash-41-sirsiz-20261029.bin` | Çalışan bir VeLinux kartının **16 MB SPI-NOR görüntüsünün tamamı** (çekirdek #41, açılış paketi #46). `/data` JFFS2 bölgesi `0xC00000–0xFE0000` **0xFF ile silinmiştir**: anahtar, jeton ya da Wi-Fi ayarı yok. | kendi derlememiz + özgün boot0 / UDISK |
| `flash-backup/sd_fel_switcher.bin` | **SD FEL anahtarı**: SD kartın 16..47. sektörlerine yazılan 16 KB. Bu kart takılıyken kart, flash açılmasa bile güç verildiğinde USB FEL kipine girer. | kendi |
| `flash-backup/sd-ilk-1MB-*.bin` | Cihazda kullanılan SD kartın ilk 1 MB'ı (MBR + boşluk) | kendi |
| `flash-package-46/` | #46 açılış paketinin öğeleri: `item-melis.bin` = **Linux 5.4 çekirdeği**, `item-rtos.bin` = **initramfs** (`F133INRD` başlığı), `item-uboot.bin` = köprü + DTB, `item-opensbi.bin` = OpenSBI, `toc1.bin` = birleştirilmiş paket, `board.dtb`. (Öğe adları özgün TOC düzeninden korunmuştur.) | kendi derlememiz |
| `kernel-v7-config/` | Önceki bir paketin (v7: ses + zram) çekirdek ayarı, DTS ve ayar farkları | kendi |
| `tina-opensbi/` | sun20iw1p1 için OpenSBI, giriş/köprü parçaları ve Linux'u **FEL ile RAM'den açmak** için kullanılan DTB (flash'a yazmadan önce çekirdek denemek için) | Tina SDK (OpenSBI: BSD-2) + kendi parçalarımız |
| `vendor/ivt244-unpacked/` | Özgün Melis firmware'i, açılmış hâli: U-Boot açılış paketi, `sys_config`, bölüm tablosu, Melis paketi | cihaz üreticisi |
| `vendor/minfs-decompressed/` | Özgün Melis kök dosya sisteminin (MINFS) içeriği: `init.axf` (ana uygulama, ELF kopyasıyla), `desktop.mod`, `update.mod`, `Config.ini` | cihaz üreticisi |
| `vendor/b5300-original/` | Özgün ayar dosyaları ve özgün firmware'in normal açılış günlüğü | cihaz üreticisi |
| `vendor/tina-reference/` | F133 için Allwinner **Tina SDK** referansı: boot0 / OpenSBI / U-Boot ikilileri, EVB kart DTS'i, `sys_config`, çekirdek ayarları, bölüm ve görüntü ayarları, Wi-Fi platform kodu | Allwinner Tina SDK |

Sağlama toplamları: `flash-backup/` ve `flash-package-46/` içindeki `SHA256SUMS` (`sha256sum -c SHA256SUMS`).

## Flash haritası (16 MB SPI-NOR)

| Adres | Boyut | İçerik |
|---|---|---|
| `0x000000` | 64 KB | boot0 (eGON.BT0) |
| `0x010000` | ~12 MB | TOC1 açılış paketi: OpenSBI, U-Boot köprüsü + DTB, Linux çekirdeği, initramfs |
| `0xC00000` | ~3,9 MB | `/data` (JFFS2): anahtarlar, Wi-Fi, ayarlar — **yedekte silinmiştir** |
| `0xFE0000` | 128 KB | UDISK (özgün firmware'den kalan küçük FAT) |

## Kurtarma

Kart artık açılmıyorsa:

1. **PC'de SD FEL anahtarı hazırlayın** (yönetici yetkisiyle):
   `python pc/sd_fel_anahtar_pc_v1.py <disk_no> yaz` — `sd_fel_switcher.bin`'i o diskin 16..47. sektörlerine yazar.
2. **FEL'e girin:** kartı cihaza takın, gücü kesip verin. Cihaz USB FEL aygıtı olarak görünür.
3. **Yalnız açılış paketini yazın** (`/data` korunur):
   `python pc/fel_flash_linux_v7.py yaz --dizin firmware/flash-package-46`
   Araç önce flash'ı yedekler, çift okumayla doğrular ve başlık sektörünü en son yazar.
4. **Anahtarı kaldırın:** `python pc/sd_fel_anahtar_pc_v1.py <disk_no> sil`, kartı geri takın, gücü kesip verin.

`flash-41` görüntüsünün tamamını `xfel spinor write 0 flash-41-sirsiz-20261029.bin` ile yazmak da çalışır, ama `/data`'yı
siler (anahtarlar ve Wi-Fi yeniden yazılmalıdır) — yalnız son çare olarak kullanın.

**İpuçları**
- Kartın Bluetooth/ses MCU'su uyanıksa ve cevap alamazsa FEL'e girdikten ~10 sn sonra işlemciyi resetler; uzun FEL
  işlerinden önce MCU'yu susturun (onunla konuşan hizmetleri durdurun).
- FEL ile RAM'den açılan bir çekirdek flash'tan yine de açılmayabilir. Flash'a yazmadan önce SD FEL anahtarını her
  zaman hazır bulundurun.

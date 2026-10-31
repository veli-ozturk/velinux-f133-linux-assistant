# VeLinux KÖLE — F133 Linux home assistant

Custom Linux for an Allwinner **F133 (RISC-V, 64 MB RAM)** smart-display board, turned into a voice / Telegram
home assistant with a camera, a touch panel UI, a web dashboard and a PC "second screen" receiver.
Everything here is the **latest version of each component** from a private working repository; personal data,
measurement logs, vendor firmware dumps and flash backups are not included.

> Türkçe açıklama aşağıda. Code comments and identifiers are mostly Turkish.

## What is in it

| Folder | Content |
|---|---|
| `go/kole-gemini/` | Go assistant (standard library only, static RISC-V build): Gemini Q&A and TTS, "Hey Köle" wake word, Telegram bot (menu, photos, alerts, allow-list for extra users), weather/currency/news board, health and OOM alerts |
| `device/src/` | C services: supervisor (`kole_gozetici`), watchdog (`kole_wdt`), Telegram long-poll over BearSSL (`kole_telgraf`), HTTPS device hub + PWA (`kole_cihaz_merkez`), camera recorder / RTSP (`kamera_kayit`), PC second-screen H.264 receiver using the F133 video engine (`wifi_ekran`), MCU bridge and more |
| `device/initramfs/` | `init` and boot helpers (Wi-Fi start with power-dip guard, FEL entry, udhcpc script) |
| `device/sd/` | SD card scripts: `autorun_vNN.sh`, Tailscale start (small-memory build), Wi-Fi watchdog, memory tuning |
| `panel/` | Touch panel UI application (framebuffer / G2D) |
| `kernel/` | Kernel config, device trees, small driver overlay |
| `patches/tailscale/` | Tailscale watchdog patch for low-memory devices |
| `scripts/` | Reference build scripts (WSL Ubuntu 22.04, RISC-V cross toolchain); paths follow the author's layout, set `VELINUX_ROOT` |
| `pc/` | Windows/PC tools: serial shell, install over serial, safe reboot, FEL SPI-NOR flasher (with tests), SD FEL switch, Telegram notifier |

## Notes

- Secrets (API keys, bot token, passwords) are never in the source: they are read at runtime from `/data/...` files
  on the device (mode 0600).
- Files are versioned by name (`name_vN`): a changed file installed on the device always gets a new name, so the
  previous version stays as a fallback.
- `pc/` tools need `pyserial` (`pip install pyserial`); the original tree vendored it under `work/serial-python-libs`.
- This is a hobby project for one specific board; it will need adapting for other hardware.

## License

MIT — see [LICENSE](LICENSE). Third-party components (Linux, BearSSL, Tailscale, OpenSBI, vendor drivers) keep
their own licenses and are not redistributed here.

---

# VeLinux KÖLE — F133 Linux ev asistanı (Türkçe)

Allwinner **F133 (RISC-V, 64 MB RAM)** akıllı ekran kartı için özel Linux. Kart; sesli ve Telegram'dan
konuşulan bir ev asistanına, kameraya, dokunmatik panele, web Merkez'e ve PC ikinci ekran alıcısına dönüştürüldü.
Burada özel çalışma deposundaki **her bileşenin yalnız son sürümü** var; kişisel veriler, ölçüm günlükleri,
üretici firmware dökümleri ve flash yedekleri yok.

- **Go asistan** (`go/kole-gemini`): Gemini ile soru-cevap ve seslendirme, "Hey Köle" uyandırma, Telegram botu
  (menü, fotoğraf, uyarılar, ek kullanıcı izin listesi), hava/döviz/haber panosu, sağlık ve bellek uyarıları.
- **C hizmetleri** (`device/src`): gözetici, bekçi köpeği, BearSSL ile Telegram, HTTPS Merkez + PWA, kamera kaydı
  ve RTSP, donanım H.264 çözücülü PC ikinci ekran alıcısı.
- **Açılış ve SD betikleri** (`device/`), **panel arayüzü** (`panel/`), **çekirdek ayarı** (`kernel/`),
  **derleme betikleri** (`scripts/`, WSL; `VELINUX_ROOT` ayarlayın), **PC araçları** (`pc/`).

Sırlar kaynakta yoktur; cihazda `/data/...` dosyalarından (0600) okunur. Lisans: MIT.

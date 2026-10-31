#!/bin/sh
# RAM proof only: keep packaged panel 0.16; SD autorun/panel never modified.
D=/mnt/sd/velinux
exec >>/tmp/autorun.log 2>&1
echo "RAM autorun g2d-zram-v1: $(cat /proc/uptime); packaged panel retained"
if [ -x "$D/wifi-ekran-v10" ]; then
    ( while true; do
        "$D/wifi-ekran-v10" --port 8766 --seconds 3600 >>/tmp/wifi-ekran.log 2>&1
        echo "wifi-ekran-v10 exited ($?)" >>/tmp/wifi-ekran.log
        sleep 2
      done ) &
fi
# O_RDONLY observer, never run concurrently with mcu or a second observer.
if [ -x "$D/mcu-izle" ]; then
    ( sleep 12
      if pidof mcu mcu-izle >/dev/null; then
          echo 'observer skipped: another MCU reader'
      else
          cp "$D/mcu-izle" /tmp/mcu-izle && chmod +x /tmp/mcu-izle &&
              /tmp/mcu-izle --seconds 0 >/dev/null 2>&1 &
      fi ) &
fi

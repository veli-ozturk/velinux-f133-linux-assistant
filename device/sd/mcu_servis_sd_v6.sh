#!/bin/sh
# v6 (24.10.2026 Claude, scripts/628, denetim-2026-10-24-v3 A7-A9/A19/A20): ikili mcu-servis-v7 (SHA7) varsa o; yoksa
#   ya da SHA'si tutmazsa mcu-servis-v6 (SHA6; kalp hic durmasin). Ikili yok/SHA tutmuyorsa CIKMAZ (v5 donguden
#   once exit 1 yapiyordu): dongu 5 sn'de bir yeniden denetler, nedeni (yok / sha) degistikce gunluge yazar.
#   Yabanci okuyucu listesinde mcu-servis-v7. --now/--erken ayni.
# v5 (23.10.2026 Claude, scripts/609, denetim v2 R1): v4 + SHA her baslatmadan once (dongu yeniden baslatirken de);
#   tutmazsa calistirilmaz, 5 sn sonra yeniden denetlenir (inceleme 24.10).
# v4 (23.10.2026 Claude, scripts/595, denetim bulgu 18): v3 + SHA uyusmazligi gunluge yazilir (sessiz cikis kalbi
#   durdurup MCU sifirlamasini izsiz birakirdi).
# v3 (23.10.2026 Claude, scripts/582): v2 + --erken (autorun_v42 en basta cagirir): ilk bekleme YOK ve yabanci okuyucu
#   denetiminde `mcu` YOK (autorun_v42 /usr/bin/mcu'yu FIFO sarmalayicisi yapar; gercek `mcu ad`i kendisi durdurur).
#   Neden: yeniden baslatmada uyanik MCU B2 kalbi ~9-10 sn cevapsiz kalinca karti sifirliyordu; v2 ~18. sn'de basliyordu.
# v2 (19.10.2026 Claude, scripts/298): v1 + ikili mcu-servis-v6 (AA DATA7 bit0=0 / B8 03 DATA2 bit7=1 RED:
# hoparloru kalici susturan A3/A4 yazimlari). Gerisi v1 ile ayni.
# Single UART owner supervisor; no explicit startup/source/volume commands.
# Default handshake gate is closed. Heartbeat replies remain enabled.
# --now is only for a measured, PID-guarded handoff; normal boot waits 12..30s.
D=/mnt/sd/velinux
LOG=/tmp/mcu-servis-sd.log
SHA7=67520f9369f69fb97bcec8c5d3136df1ff0928732fe9c8fde2570544436be925
SHA6=df95a8434dafce53a492817be59d034882f4a81935fb4886f3ef5fcb66e15aef
case "$1" in ''|--now|--erken) ;; *) echo 'usage: mcu_servis_sd_v6.sh [--now|--erken]'; exit 2;; esac
YABANCI='mcu mcu-izle mcu-servis mcu-servis2 mcu-servis3 mcu-servis-v3 mcu-servis-v6 mcu-servis-v7'	# v6: v7
[ "$1" = --erken ] && YABANCI='mcu-izle mcu-servis mcu-servis2 mcu-servis3 mcu-servis-v3 mcu-servis-v6 mcu-servis-v7'	# v3, v6
exec 9>/tmp/mcu-servis-sd.lock
flock -n 9 || exit 0
dene() {	# v6: dene <surum> <sha>: calistirilabilir ve SHA tutuyorsa BIN; degilse NEDEN'e eklenir
    BIN=$D/mcu-servis-$1
    if ! test -x "$BIN"; then NEDEN="$NEDEN $1 yok;"; return 1; fi
    test "$(sha256sum "$BIN" | awk '{print $1}')" = "$2" && return 0
    NEDEN="$NEDEN $1 sha uyusmuyor;"
    return 1
}
echo $$ >/tmp/mcu-servis-sd.pid
child=
guard=
cleanup() {
    trap - EXIT TERM INT
    test -z "$child" || kill "$child" 2>/dev/null
    test -z "$guard" || kill "$guard" 2>/dev/null
    test -z "$child" || wait "$child" 2>/dev/null
    rm -f /tmp/mcu-servis-sd.pid
}
trap cleanup EXIT
trap 'exit 0' TERM INT
# Keep at most 64KiB current plus 32KiB preceding tail (RAM only).
# Truncate the open inode so the running service keeps its stdout fd valid.
(
    exec 9>&-
    while true; do
        if [ -f "$LOG" ] && [ "$(wc -c <"$LOG")" -gt 65536 ]; then
            tail -c 32768 "$LOG" >"$LOG.previous.new" &&
                mv "$LOG.previous.new" "$LOG.previous" && : >"$LOG"
        fi
        sleep 30
    done
) </dev/null >/dev/null 2>&1 &
guard=$!
if [ "$1" != --now ] && [ "$1" != --erken ]; then	# v3
    i=0
    while [ "$i" -lt 30 ] && { [ "$i" -lt 12 ] || pidof mcu >/dev/null; }; do
        sleep 1
        i=$((i+1))
    done
fi
while true; do
    # Never evict a foreign reader. The C service also uses the existing UART flock.
    if pidof $YABANCI >/dev/null; then	# v3
        sleep 2
        continue
    fi
    NEDEN=
    if ! { dene v7 "$SHA7" || dene v6 "$SHA6"; }; then	# v5/v6: calistirilmaz, 5 sn sonra yeniden denetim
        [ "$u" = "$NEDEN" ] || echo "SD mcu-servis:$NEDEN kalp YOK; 5 sn'de bir denetlenir" >>"$LOG"
        u=$NEDEN
        sleep 5
        continue
    fi
    u=
    echo "SD ${BIN##*/} starting: $(cat /proc/uptime), handshake=0 ${1:-}${NEDEN:+ (atlandi:$NEDEN)}" >>"$LOG"
    MCU_ELSIKISMA=0 "$BIN" 9>&- </dev/null >>"$LOG" 2>&1 &
    child=$!
    echo "$child" >/tmp/mcu-servis3.pid
    wait "$child"
    rc=$?
    child=
    echo "SD ${BIN##*/} exit=$rc, restart after 1s" >>"$LOG"
    sleep 1
done

#!/bin/sh
# kamera_kopru_sd_v12 (24.10.2026, Claude; scripts/630, denetim v3 A6): v11 + RTSP bayragi acilista yoksa 5 sn'de bir
#   bakan uyuyan kabuk (bayrak gelince RTSP baslar; v11'de "RTSP ac" yeniden baslatmaya kadar islemiyordu);
#   kole-kopru-v12 > v11 > v9; kamera-kayit-v12 once.
# kamera_kopru_sd_v11 (24.10.2026, Claude; scripts/618): v10, kamera-kayit-v11 once (hareket varken 8 kare/sn kayit).
# kamera_kopru_sd_v10 (23.10.2026, Claude): v9, kamera-kayit-v10 once (sonra v9..v3, v2); kole-kopru-v11 calistirilabilirse
#   o, yoksa kole-kopru-v9 (sunucu + rtsp ayni ikili); pidof listesine yeni adlar.
# kamera_kopru_sd_v9 (15.10.2026, Claude): v8, en yeni kamera-kayit genel secim (v9..v3, v2).
# kamera_kopru_sd_v8 (15.10.2026, Claude): v7, en yeni kamera-kayit (v6: kamera saati + USB kurtarma; yoksa v3, v2).
# kamera_kopru_sd_v7 (15.10.2026, Claude): v6, kamera-kayit-v3 (bekci/foto/zaman atlamali; yoksa v2).
# kamera_kopru_sd_v6 (15.10.2026, Claude): v5, kole-kopru-v8 yerine v9 (ses_akis_v2: Hey Kole mikrofonu birakir).
# kamera_kopru_sd_v5 (14.10.2026, Claude): v3 + RTSP sureci YALNIZ $D/rtsp-acik varsa (bellek).
# --- v3 ---
# kamera_kopru_sd_v3 (13.10.2026, Claude): v2 + kole-kopru v8 (canli ses, bas-konus) + RTSP sureci.
# Tek kamera ve kopru sahibi; MCU/panel/Wi-Fi hizmetlerine dokunmaz. HENUZ KURULMADI (cihaz erisimi yoktu):
# kurulum adimlari scripts/135_prepare_kole_kopru_v8_sd.sh ciktisinda.
#   kamera-kayit-v2           : kameranin TEK sahibi (yalniz READ(10))
#   kole-kopru-v8 sunucu 8767 : anahtarli kopru (KOLE Merkez)
#   kole-kopru-v8 rtsp 8554   : RTSP; /mnt/sd/velinux/rtsp-acik ve /data/rtsp-parola YOKSA PORT ACMAZ (varsayilan kapali)
set -u
D=/mnt/sd/velinux
L=/tmp/kamera-kopru-sd.lock
P=/tmp/kamera-kopru-sd.pid
if ! mkdir "$L" 2>/dev/null; then
    p=$(cat "$P" 2>/dev/null)
    if [ -z "$p" ]; then echo "Kilit var, PID henuz yok; baslatilmadi"; exit 1; fi
    if kill -0 "$p" 2>/dev/null; then
        echo "kamera/kopru gozeticisi zaten acik: $p"
        exit 0
    fi
    rmdir "$L" 2>/dev/null || exit 1
    mkdir "$L" || exit 1
fi
echo $$ > "$P"
KLOOP= BLOOP= RLOOP=
temizle() {
    trap '' TERM INT EXIT
    for p in "$KLOOP" "$BLOOP" "$RLOOP"; do
        [ -n "$p" ] && kill "$p" 2>/dev/null
    done
    rm -f "$P" /tmp/kamera-kopru-camera-loop.pid /tmp/kamera-kopru-bridge-loop.pid /tmp/kamera-kopru-rtsp-loop.pid
    rmdir "$L" 2>/dev/null
    exit 0
}
trap temizle TERM INT EXIT
dongu() {
    ad=$1
    log=$2
    shift 2
    while true; do
        if [ -f "$log" ] && [ "$(wc -c < "$log")" -gt 65536 ]; then
            tail -c 32768 "$log" > "$log.kirp" && mv "$log.kirp" "$log"
        fi
        "$D/$ad" "$@" </dev/null >>"$log" 2>&1 &
        child=$!
        trap 'kill "$child" 2>/dev/null; exit 0' TERM INT
        wait "$child"
        rc=$?
        echo "$ad cikti=$rc; 2 sn sonra yeniden" >>"$log"
        sleep 2
    done
}
KK=kamera-kayit-v2
for v in kamera-kayit-v12 kamera-kayit-v11 kamera-kayit-v10 kamera-kayit-v9 kamera-kayit-v8 kamera-kayit-v7 kamera-kayit-v6 kamera-kayit-v5 kamera-kayit-v4 kamera-kayit-v3; do
    if [ -x "$D/$v" ]; then KK=$v; break; fi
done
KP=kole-kopru-v9
[ -x "$D/kole-kopru-v11" ] && KP=kole-kopru-v11
[ -x "$D/kole-kopru-v12" ] && KP=kole-kopru-v12	# v12
if [ ! -x "$D/$KK" ] || [ ! -x "$D/$KP" ]; then
    echo "kamera/kopru SD ikilisi eksik ($KK, $KP)"
    temizle
fi
if pidof kamera-kayit-v1 kamera-kayit-v2 kamera-kayit-v3 kamera-kayit-v4 kamera-kayit-v5 kamera-kayit-v6 kamera-kayit-v7 kamera-kayit-v8 kamera-kayit-v9 kamera-kayit-v10 kamera-kayit-v11 kamera-kayit-v12 kole-kopru-v6 kole-kopru-v7 kole-kopru-v8 kole-kopru-v9 kole-kopru-v10 kole-kopru-v11 kole-kopru-v12 >/dev/null 2>&1; then
    echo "Mevcut okuyucu/kopru var; ikinci sahip baslatilmadi"
    temizle
fi
dongu $KK /tmp/kamera-kayit-sd.log &
KLOOP=$!
echo "$KLOOP" > /tmp/kamera-kopru-camera-loop.pid
dongu $KP /tmp/kole-kopru-sd.log sunucu 8767 &
BLOOP=$!
echo "$BLOOP" > /tmp/kamera-kopru-bridge-loop.pid
if [ -f "$D/rtsp-acik" ]; then
    dongu $KP /tmp/kole-rtsp-sd.log rtsp 8554 &
    RLOOP=$!
    echo "$RLOOP" > /tmp/kamera-kopru-rtsp-loop.pid
else
    echo "RTSP bayragi yok: RTSP sureci bayrak gelince baslayacak (~13 MB bellek tasarrufu)"
    ( while [ ! -f "$D/rtsp-acik" ]; do sleep 5; done; dongu $KP /tmp/kole-rtsp-sd.log rtsp 8554 ) &	# v12
    RLOOP=$!
    echo "$RLOOP" > /tmp/kamera-kopru-rtsp-loop.pid
fi
echo "kamera/kopru gozetici=$$ kamera-loop=$KLOOP kopru-loop=$BLOOP rtsp-loop=$RLOOP"
wait

#!/bin/bash
# 663 (26.10.2026, Claude): v4 = v3 + bosta kapanan Merkez'e yeniden baglanma. (661: v3 = v2 + baglanti gunlugu.) kole-tlsvekil-v4 (PROXY v1 + guvenli X-Forwarded-* / Tailscale-User-Login). Host: normal +
#   ASAN, EC ve RSA sahte sertifikayla 663_tlsvekil_v4_test.py (PROXY, taklit baslik, keep-alive, chunked, Upgrade, whois).
#   Cikti ~/f133/out/kole-tlsvekil-v4/ ve ana klasor work/terminal-local/ota-068/kole-tlsvekil-v4/. CIHAZA GONDERMEZ.
set -euo pipefail
P="${P:-$HOME/f133/proj}"
B="$HOME/f133/out/bearssl-0.6"
I=(-I"$HOME/f133/bearssl-kaynak/bearssl-0.6/inc" -I"$B")
O="$HOME/f133/out/kole-tlsvekil-v4"
S="$P/initramfs/src/kole_tlsvekil_v4.c"
mkdir -p "$O"
gcc -O2 -Wall -Wextra -Werror "${I[@]}" "$S" "$B/libbearssl-host.a" -o "$O/tlsvekil-host"
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "${I[@]}" "$S" "$B/libbearssl-host.a" \
	-o "$O/tlsvekil-host-asan"
T=$(mktemp -d)
trap 'kill $(jobs -p) 2>/dev/null; rm -rf "$T"' EXIT
openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:P-256 -nodes -keyout "$T/ec.key" -out "$T/ec.crt" -days 2 -subj /CN=localhost 2>/dev/null
openssl req -x509 -newkey rsa:2048 -nodes -keyout "$T/rsa.key" -out "$T/rsa.crt" -days 2 -subj /CN=localhost 2>/dev/null
for k in ec rsa; do
	"$O/tlsvekil-host-asan" 18443 127.0.0.1 18780 "$T/$k.crt" "$T/$k.key" > "$T/vekil-$k.log" 2>&1 &
	V=$!
	sleep 1
	if ! python3 "$P/scripts/663_tlsvekil_v4_test.py" 18443 18780; then kill $V; cat "$T/vekil-$k.log"; exit 1; fi
	kill $V; wait $V 2>/dev/null || true
	if grep -q -e "ERROR: AddressSanitizer" -e "runtime error" "$T/vekil-$k.log"; then echo "HATA: $k ASAN"; cat "$T/vekil-$k.log"; exit 1; fi
	echo "host $k: tamam ($(head -1 "$T/vekil-$k.log"))"
	grep -q "istek 2 yeniden 1 tls_hata 0" "$T/vekil-$k.log" && grep -q "istek 3 yeniden 0 tls_hata 0" "$T/vekil-$k.log" || { echo "HATA: gunluk satiri yok"; cat "$T/vekil-$k.log"; exit 1; }; tail -4 "$T/vekil-$k.log"
	sleep 1	# arka uc portu (TIME_WAIT degil, SO_REUSEADDR) serbest kalsin
done
if "$O/tlsvekil-host" 18444 127.0.0.1 18780 "$T/ec.crt" "$T/yok.key" 2>/dev/null; then echo "HATA: anahtarsiz basladi"; exit 1; fi
echo "host: anahtar yokken reddetti"
riscv64-linux-gnu-gcc -static -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wall -Wextra -Werror "${I[@]}" "$S" \
	"$B/libbearssl-riscv.a" -o "$O/kole-tlsvekil-v4"
riscv64-linux-gnu-strip "$O/kole-tlsvekil-v4"
file "$O/kole-tlsvekil-v4"
sha256sum "$O/kole-tlsvekil-v4"
stat -c 'bytes=%s' "$O/kole-tlsvekil-v4"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-tlsvekil-v4"
mkdir -p "$H" && cp "$O/kole-tlsvekil-v4" "$H/" && echo "kopyalandi: $H"
echo "663 TAMAM"

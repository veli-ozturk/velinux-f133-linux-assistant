#!/bin/bash
# 674 (30.10.2026, Claude; kullanıcı: esi de bota): kole-telgraf-v7 (izinli liste). 669 kalibi.
set -euo pipefail
P="${P:-$HOME/f133/proj}"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068"
A=(-O2 -Wall -Wextra -Werror)
ASAN=(-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror)
cik() {	# ad kaynak [ek bayraklar...]
	local ad=$1 kaynak=$2 O="$HOME/f133/out/$1"
	shift 2
	riscv64-linux-gnu-strip "$O/$ad"
	file "$O/$ad"
	sha256sum "$O/$ad" | tee "$O/$ad.sha256"
	stat -c 'bytes=%s' "$O/$ad"
	mkdir -p "$H/$ad" && cp "$O/$ad" "$O/$ad.sha256" "$H/$ad/" && echo "kopyalandi: $H/$ad"
}
echo "== kole-telgraf-v7"
B="$HOME/f133/out/bearssl-0.6"
I=(-I"$HOME/f133/bearssl-kaynak/bearssl-0.6/inc" -I"$B")
O="$HOME/f133/out/kole-telgraf-v7"
mkdir -p "$O"
S="$P/initramfs/src/kole_telgraf_v7.c"
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
printf '#!/bin/sh\ncat "$2" > "$KT_SAHTE_CIKTI"; rm -f "$2"\necho hazir >&3\nexec 3>&-\nsleep 2\n' > "$T/sahte-go"
chmod 755 "$T/sahte-go"
gcc "${A[@]}" "${I[@]}" "$S" "$B/libbearssl-host.a" -o "$O/kole-telgraf-host"
KT_SAHTE_GO="$T/sahte-go" KT_SAHTE_CIKTI="$T/cikti" "$O/kole-telgraf-host" --self-test
gcc "${ASAN[@]}" "${I[@]}" "$S" "$B/libbearssl-host.a" -o "$O/kole-telgraf-host-asan"
KT_SAHTE_GO="$T/sahte-go" KT_SAHTE_CIKTI="$T/cikti" "$O/kole-telgraf-host-asan" --self-test | sed 's/^/[asan] /'
riscv64-linux-gnu-gcc -static -Os -ffunction-sections -fdata-sections -Wl,--gc-sections "${A[@]}" "${I[@]}" "$S" \
	"$B/libbearssl-riscv.a" -o "$O/kole-telgraf-v7"
cik kole-telgraf-v7 "$S"
echo "674 TAMAM"

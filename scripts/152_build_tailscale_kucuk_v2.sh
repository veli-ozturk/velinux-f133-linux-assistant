#!/bin/bash
# 152 (14.10.2026, Claude; v2: yama watchdog_yama_v2.py ile): VeLinux icin kucuk, tek dosya (box) Tailscale v1.104.1 riscv64 derlemesi.
#  - kaynak: ~/ts-derle/tailscale (git clone --depth 1 --branch v1.104.1, commit 9a522a97)
#  - Go: ~/ts-derle/go (go1.27.1 resmi, sha256 63d339f0...)
#  - yama: linux-build/tailscale-yama/watchdog_yama_v2.py (diff: watchdog-v2.diff) (kilit bekcisi panik yerine gunluk)
#  - ozellikler: featuretags --min + netstack,serve,acme,nattraversal,udptransport,ipnbus,bakedroots,portmapper,health
#    + ts_include_cli (TS_BE_CLI=1 ile komut satiri). Cikti: linux-build/out/tailscaled-kucuk
set -euo pipefail
P=/home/veli/f133/proj
S=$HOME/ts-derle/tailscale
export PATH="$HOME/ts-derle/go/bin:/usr/local/bin:/usr/bin:/bin" GOTOOLCHAIN=local
cd "$S"
test "$(git rev-parse HEAD)" = 9a522a9786c97eb7910c01ccb7bd66557b04c910
python3 "$P/tailscale-yama/watchdog_yama_v2.py" "$S"
git diff > "$P/tailscale-yama/watchdog-v2.diff"; git diff --stat
grep -q 'panik devre disi (VeLinux yamasi)' ipn/ipnlocal/watchdog.go
! grep -q 'panic("ipnlocal: watchdog timeout")' ipn/ipnlocal/watchdog.go
OZ=netstack,serve,acme,nattraversal,udptransport,ipnbus,bakedroots,portmapper,health
TAGS="$(go run ./cmd/featuretags --min --add=$OZ),ts_include_cli"
echo "etiket sayisi: $(echo "$TAGS" | tr ',' '\n' | wc -l)"
GOOS=linux GOARCH=riscv64 CGO_ENABLED=0 TAGS="$TAGS" ./build_dist.sh --strip -o "$P/out/tailscaled-kucuk" ./cmd/tailscaled
file "$P/out/tailscaled-kucuk"
ls -l "$P/out/tailscaled-kucuk"
sha256sum "$P/out/tailscaled-kucuk"

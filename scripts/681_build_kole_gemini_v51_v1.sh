#!/bin/bash
# 681 (31.10.2026, Claude): kole-gemini v51 = onceki + duz cevaplar kalici menuyle. 680 kalibi.
#   CIHAZA GONDERMEZ. --- 639 ---
# 599 (23.10.2026, Claude, 587 kalibi): kole-gemini v42 (denetim v2 düzeltmeleri #5 #8 #10 #11 #17 #18 #20 R2 + canlı
# uzatma sınırı; scripts/598). P = ağaç kökü (linux-build; worktree için ~/f133/wt), yoksa ~/f133/proj.
# Cikti ana klasor work/terminal-local/ota-068/kole-gemini-v42/ de. CIHAZA GONDERMEZ. --- 587 ---
# 587 (23.10.2026, Claude, 565 kalibi): kole-gemini v41 (Telegram tek atış --tg-isle/--tg-zamanli, canlı süreçler arası
# denetim; scripts/586). Go: ~/ts-derle/go (go1.27.1, yalniz standart kutuphane). gofmt + vet + test + -race alt kumesi,
# host (amd64) ve RISC-V statik (CGO yok).
set -euo pipefail
G=$HOME/ts-derle/go/bin
export PATH="$G:$PATH" GOFLAGS=-mod=mod GOTOOLCHAIN=local GOPROXY=off
S="${P:-$HOME/f133/proj}/go/kole-gemini-v51"
O="$HOME/f133/out/kole-gemini-v51"
mkdir -p "$O"
cd "$S"
[ -z "$(gofmt -l .)" ] || { echo "HATA: gofmt"; gofmt -d . | head -40; exit 1; }
go vet ./...
go test -count=1 ./...
R='TestCanli|TestTgBekleSn|TestTgMenu|TestYaris|TestIstem|TestModel|TestHaberOnbellek|TestEylemSes|TestEylemEnum|TestZaman|TestEylemZaman|TestTgMenuZaman|TestHeyKole|TestEylemHeyKole|TestTgMenuHeyKole|TestOom|TestSayacOomKurban|TestEleven402|TestAkisAtla|TestSoketSor|TestSoruSunucusu|TestSoruIsle|TestTgIsle|TestTgHazir|TestCanliBaska|TestCanliKontrol|TestDenetim|TestIzinli|TestJp|TestJpKisayol'
go test -count=1 -race -run "$R" -v ./... > "$O/race.log" 2>&1 || { grep -E '^(=== RUN|--- |ok|FAIL|WARNING)' "$O/race.log"; echo 'HATA: -race'; exit 1; }
grep -E '^(--- |ok|FAIL|WARNING)' "$O/race.log"
CGO_ENABLED=0 go build -trimpath -ldflags "-s -w" -o "$O/kole-gemini-host" .
CGO_ENABLED=0 GOOS=linux GOARCH=riscv64 go build -trimpath -ldflags "-s -w" -o "$O/kole-gemini-v51" .
file "$O/kole-gemini-v51"
sha256sum "$O/kole-gemini-v51" | tee "$O/kole-gemini-v51.sha256"
stat -c 'bytes=%s' "$O/kole-gemini-v51"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-gemini-v51"
mkdir -p "$H" && cp "$O/kole-gemini-v51" "$H/" && echo "kopyalandi: $H"

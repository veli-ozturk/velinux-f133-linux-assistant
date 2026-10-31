// butce.go (20.10.2026, Claude): v21 süre bütçesi (scripts/396). Asistan kole-gemini'yi 30 sn'de SIGKILL ile keser
// (kole_asistan_v11), panel --metin için 45 sn bekler. JSON bu sürelerden önce basılmazsa hazır METİN cevap da kaybolur
// (20.10 22:00: 3.8-flash 16.6 sn + TTS beklemesi -> "cevap çok gecikti").
package main

import "time"

const (
	sesliCikisSiniri = 27 * time.Second // asistan 30 sn - 3 sn pay
	metinCikisSiniri = 42 * time.Second // panel 45 sn - 3 sn pay
)

var (
	surecBas    = time.Now()
	cikisSiniri = sesliCikisSiniri
)

// süreç başından cikisSiniri'ne kalan süre, en çok azami, en az 0
func butce(azami time.Duration) time.Duration {
	k := cikisSiniri - time.Since(surecBas)
	if k > azami {
		k = azami
	}
	if k < 0 {
		k = 0
	}
	return k
}

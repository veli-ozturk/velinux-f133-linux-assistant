package main

// v44 (28.10.2026, Claude; docs §188): TTS'in konuşmanın başına/sonuna eklediği sessizlik kırpılır (Hey Köle karşılaması
// 0.75 sn konuşma için 2.6 sn sürüyordu). Amfi için 1 sn ön sessizlik pcmHazirla'da korunur.

import (
	"encoding/binary"
	"os"
	"path/filepath"
)

const (
	kirpEsik   = 300             // |örnek| bunun altı sessiz sayılır
	kirpOnPay  = ttsHedefHz / 20 // konuşmadan önce 50 ms bırak (ilk hece yumuşak başlasın)
	kirpSonPay = ttsHedefHz / 10 // konuşmadan sonra 100 ms bırak
	kirpIsaret = ".kirpildi-v44" // hazır ses dizininde bir kerelik göç işareti
)

// sessizKirp: baştaki ve sondaki sessizliği payları bırakarak atar; tamamen sessizse dokunmaz.
func sessizKirp(x []int16) []int16 {
	a, b := 0, len(x)
	for a < b && x[a] <= kirpEsik && x[a] >= -kirpEsik {
		a++
	}
	for b > a && x[b-1] <= kirpEsik && x[b-1] >= -kirpEsik {
		b--
	}
	if a == b {
		return x
	}
	a = max(a-kirpOnPay, 0)
	b = min(b+kirpSonPay, len(x))
	return x[a:b]
}

// sessizGoc: v43 ve öncesinin ürettiği hazır sesleri ve selamı bir kez yerinde kırpar (TTS'e yeni istek yok, ses aynı).
// Kırpılan dosya sayısını döndürür; işaret varsa -1.
func sessizGoc(dizin, selam string) int {
	isaret := filepath.Join(dizin, kirpIsaret)
	if dosyaVar(isaret) {
		return -1
	}
	yollar, _ := filepath.Glob(filepath.Join(dizin, "*.pcm"))
	if selam != "" {
		yollar = append(yollar, selam)
	}
	n := 0
	for _, y := range yollar {
		b, err := os.ReadFile(y)
		if err != nil || len(b) < 3200 || len(b)%2 != 0 {
			continue
		}
		x := make([]int16, len(b)/2)
		for i := range x {
			x[i] = int16(binary.LittleEndian.Uint16(b[2*i:]))
		}
		yeni := pcmHazirla(x) // içinde sessizKirp: eski 1 sn ön + TTS sessizliği atılır, 1 sn ön yeniden eklenir
		if len(yeni) >= len(b) {
			continue
		}
		if dosyaYazAtomik(y, yeni, 0644) == nil {
			n++
		}
	}
	_ = dosyaYazAtomik(isaret, []byte("v44\n"), 0644)
	return n
}

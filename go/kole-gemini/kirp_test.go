package main

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"
)

func sesYap(onSessiz, konusma, sonSessiz int) []int16 {
	x := make([]int16, onSessiz+konusma+sonSessiz)
	for i := 0; i < konusma; i++ {
		x[onSessiz+i] = 4000
		if i%2 == 1 {
			x[onSessiz+i] = -4000
		}
	}
	return x
}

func TestSessizKirp(t *testing.T) {
	x := sesYap(4320, 12000, 5920) // TTS: 0.27 sn önde, 0.75 sn konuşma, 0.37 sn sonda
	k := sessizKirp(x)
	if want := kirpOnPay + 12000 + kirpSonPay; len(k) != want {
		t.Fatalf("kırpılmış %d örnek, beklenen %d", len(k), want)
	}
	if k[kirpOnPay] == 0 || k[kirpOnPay-1] != 0 {
		t.Fatal("konuşma başlangıcı korunmadı")
	}
	if s := make([]int16, 100); len(sessizKirp(s)) != 100 {
		t.Fatal("tamamen sessiz ses değişmemeli")
	}
	if len(sessizKirp(nil)) != 0 {
		t.Fatal("boş girdi")
	}
	// kenara yakın konuşma: pay dizi dışına taşmasın
	if k := sessizKirp(sesYap(10, 50, 10)); len(k) != 70 {
		t.Fatalf("kenar: %d", len(k))
	}
}

func TestPcmHazirlaKirpar(t *testing.T) {
	b := pcmHazirla(sesYap(4320, 12000, 5920))
	on, son := ttsHedefHz*onSessizMs/1000, ttsHedefHz/5
	if want := 2 * (on + kirpOnPay + 12000 + kirpSonPay + son); len(b) != want {
		t.Fatalf("pcm %d bayt, beklenen %d", len(b), want)
	}
}

func TestSessizGocBirKez(t *testing.T) {
	d := t.TempDir()
	eski := pcmHazirlaEski(sesYap(4320, 12000, 5920))
	y := filepath.Join(d, "a.pcm")
	selam := filepath.Join(d, "selam.pcm")
	_ = os.WriteFile(y, eski, 0644)
	_ = os.WriteFile(selam, eski, 0644)
	_ = os.WriteFile(filepath.Join(d, "bozuk.pcm"), []byte{1, 2, 3}, 0644)
	if n := sessizGoc(d, selam); n != 2 {
		t.Fatalf("kırpılan %d, beklenen 2", n)
	}
	b, _ := os.ReadFile(y)
	if len(b) >= len(eski) {
		t.Fatalf("dosya kısalmadı: %d >= %d", len(b), len(eski))
	}
	if len(b) != len(pcmHazirla(sesYap(4320, 12000, 5920))) {
		t.Fatal("göç sonrası boy yeni üretimle aynı olmalı")
	}
	if binary.LittleEndian.Uint16(b[0:]) != 0 {
		t.Fatal("1 sn ön sessizlik korunmalı")
	}
	if n := sessizGoc(d, selam); n != -1 {
		t.Fatalf("ikinci çağrı -1 olmalı (işaret), %d", n)
	}
}

// v43 davranışı (kırpmasız): göç testinin girdisi
func pcmHazirlaEski(x []int16) []byte {
	on, son := ttsHedefHz*onSessizMs/1000, ttsHedefHz/5
	b := make([]byte, 2*(on+len(x)+son))
	for i, v := range x {
		binary.LittleEndian.PutUint16(b[2*(on+i):], uint16(v))
	}
	return b
}

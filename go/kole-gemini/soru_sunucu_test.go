// soru_sunucu_test.go (23.10.2026, Claude; v40): soket protokolü, sunucu hata yolu, butce sıfırlama, ince istemci geri dönüşü.
package main

import (
	"bufio"
	"net"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestSoketSorProtokol(t *testing.T) {
	d := t.TempDir()
	yol := filepath.Join(d, "s.sock")
	if _, _, ok := soketSor(yol, soruIstek{Metin: "x"}, time.Second); ok {
		t.Fatal("soket yokken ok=false olmalı (çağıran kendisi yapar)")
	}
	ln, err := net.Listen("unix", yol)
	if err != nil {
		t.Skip("unix soket yok:", err)
	}
	defer ln.Close()
	cevaplar := []string{`{"soru":"x","cevap":"Evet efendim."}` + "\n", `{"rc":6}` + "\n", ""}
	go func() {
		for _, c := range cevaplar {
			k, err := ln.Accept()
			if err != nil {
				return
			}
			_, _ = bufio.NewReader(k).ReadString('\n')
			_, _ = k.Write([]byte(c))
			k.Close()
		}
	}()
	if out, rc, ok := soketSor(yol, soruIstek{Metin: "x"}, 2*time.Second); !ok || rc != 0 || !strings.Contains(out, "Evet efendim") {
		t.Fatalf("başarı: %q %d %v", out, rc, ok)
	}
	if _, rc, ok := soketSor(yol, soruIstek{Metin: "x"}, 2*time.Second); !ok || rc != 6 {
		t.Fatalf("rc: %d %v", rc, ok)
	}
	if _, rc, ok := soketSor(yol, soruIstek{Metin: "x"}, 2*time.Second); !ok || rc != 5 {
		t.Fatalf("cevapsız kapanış rc 5: %d %v", rc, ok)
	}
}

func TestSoruSunucusuHataYolu(t *testing.T) {
	d := t.TempDir()
	eski := soruSoket
	soruSoket = filepath.Join(d, "kg.sock")
	defer func() { soruSoket = eski }()
	if kapat := soruSunucusu(soruYollar{}); kapat == nil {
		t.Fatal("boş yollarla no-op kapat dönmeli")
	} else if kapat(); dosyaVar(soruSoket) {
		t.Fatal("anahtar yolu yokken sunucu açılmamalı")
	}
	y := soruYollar{anahtar: filepath.Join(d, "anahtar-yok"), gecmis: filepath.Join(d, "g.json"), eylem: filepath.Join(d, "e.json"),
		sure: time.Second}
	kapat := soruSunucusu(y)
	if fi, err := os.Stat(soruSoket); err != nil || fi.Mode().Perm() != 0600 {
		t.Fatalf("soket 0600 değil: %v %v", fi, err)
	}
	surecBas = time.Now().Add(-time.Hour) // uzun yaşayan süreç taklidi
	if _, rc, ok := soketSor(soruSoket, soruIstek{Metin: "merhaba"}, 3*time.Second); !ok || rc != 3 {
		t.Fatalf("anahtar yok -> rc 3: %d %v", rc, ok)
	}
	if time.Since(surecBas) > 5*time.Second || butce(10*time.Second) < 9*time.Second {
		t.Fatalf("butce soru başından sayılmalı: surecBas %v önce, butce %v", time.Since(surecBas), butce(10*time.Second))
	}
	if _, rc, ok := soketSor(soruSoket, soruIstek{}, 3*time.Second); !ok || rc != 2 {
		t.Fatalf("boş istek rc 2: %d %v", rc, ok)
	}
	if _, rc, ok := soketSor(soruSoket, soruIstek{Ses: filepath.Join(d, "yok.wav")}, 3*time.Second); !ok || rc != 3 {
		t.Fatalf("ses (anahtar yok) rc 3: %d %v", rc, ok) // anahtar denetimi ses okumadan önce (v39 sırası)
	}
	kapat()
	if dosyaVar(soruSoket) {
		t.Fatal("kapanınca soket dosyası silinmeli")
	}
	if _, _, ok := soketSor(soruSoket, soruIstek{Metin: "x"}, time.Second); ok {
		t.Fatal("kapandıktan sonra ok=false olmalı")
	}
}

func TestSoruIsleMetinSiniri(t *testing.T) {
	d := t.TempDir()
	y := soruYollar{anahtar: filepath.Join(d, "yok"), sure: time.Second}
	_ = os.WriteFile(y.anahtar, []byte("KEY\n"), 0600) // anahtar var, ses dosyası yok -> rc 4 (sınır ayarlandıktan sonra)
	cikisSiniri = metinCikisSiniri
	if _, rc := soruIsle(y, soruIstek{Ses: filepath.Join(d, "yok.wav")}); rc != 4 {
		t.Fatalf("rc %d", rc)
	}
	if cikisSiniri != sesliCikisSiniri {
		t.Fatal("ses sorusundan sonra sesli sınır olmalı")
	}
	cikisSiniri = sesliCikisSiniri
}

func TestSesBayat(t *testing.T) { // v43 (A20)
	yol := t.TempDir() + "/asistan-soru.wav"
	if err := os.WriteFile(yol, []byte("RIFF1"), 0600); err != nil {
		t.Fatal(err)
	}
	time.Sleep(20 * time.Millisecond)
	alindi := time.Now()
	if sesBayat(soruIstek{Ses: yol}, alindi) {
		t.Fatal("istekten önce yazılan ses bayat sayıldı")
	}
	time.Sleep(20 * time.Millisecond)
	if err := os.WriteFile(yol, []byte("RIFF2"), 0600); err != nil { // sonraki soru sabit dosyayı yeniler
		t.Fatal(err)
	}
	if !sesBayat(soruIstek{Ses: yol}, alindi) {
		t.Fatal("istekten sonra değişen ses bayat sayılmadı")
	}
	if sesBayat(soruIstek{Metin: "x"}, alindi) {
		t.Fatal("metin sorusu bayat olamaz")
	}
	if _, rc := soruIsleKilitli(soruYollar{}, soruIstek{Ses: yol}, alindi); rc != 5 {
		t.Fatalf("bayat istek rc %d, 5 bekleniyordu", rc)
	}
}

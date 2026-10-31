// zaman_kip_test.go (22.10.2026, Claude; v34): /zaman aralık + olay, durum metni, sesli eylem, ⏱ düğmeleri.
package main

import (
	"context"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"testing"
)

func geciciZaman(t *testing.T) zamanYollar {
	d := t.TempDir()
	return zamanYollar{filepath.Join(d, "zaman-acik"), filepath.Join(d, "zaman-aralik"), filepath.Join(d, "zaman-olay"),
		filepath.Join(d, "kamera-kayit.json")}
}

func TestZamanAralikCoz(t *testing.T) {
	for s, n := range map[string]int{"1": 1, "3": 3, "3sn": 3, "3 sn": 3, "10s": 10, "1dk": 60, "5 dk": 300, "3600": 3600,
		"60dk": 3600, "0": 0, "3601": 0, "61dk": 0, "-3": 0, "abc": 0, "": 0} {
		if g := zamanAralikCoz(s); g != n {
			t.Fatalf("%q: %d (beklenen %d)", s, g, n)
		}
	}
}

func TestZamanKomut(t *testing.T) {
	z := geciciZaman(t)
	oku := func() string { b, _ := os.ReadFile(z.Aralik); return string(b) }
	if c := zamanKomut(z, []string{"3"}); oku() != "3\n" || !strings.Contains(c, "3 sn oldu") || !strings.Contains(c, "Çekim kapalı") {
		t.Fatalf("3: %q %q", oku(), c)
	}
	if c := zamanKomut(z, []string{"ac"}); !dosyaVar(z.Acik) || !strings.Contains(c, "her 3 sn") {
		t.Fatalf("ac: %q", c)
	}
	zamanKomut(z, []string{"1dk"})
	if oku() != "60\n" || zamanAralikOku(z.Aralik) != 60 {
		t.Fatal("1dk")
	}
	zamanKomut(z, []string{"aralik", "10"})
	if oku() != "10\n" {
		t.Fatal("aralik 10")
	}
	if c := zamanKomut(z, []string{"0"}); c != zamanKullanim || oku() != "10\n" {
		t.Fatalf("0: %q", c)
	}
	if c := zamanKomut(z, []string{"xyz"}); c != zamanKullanim || !dosyaVar(z.Acik) {
		t.Fatal("bilinmeyen argüman aç/kapat değiştirmemeli")
	}
	if c := zamanKomut(z, []string{"olay", "ac"}); !dosyaVar(z.Olay) || strings.Contains(c, "şu an kapalı") {
		t.Fatalf("olay ac: %q", c)
	}
	if c := zamanKomut(z, []string{"ac"}); !strings.Contains(c, "olay tespiti açık") {
		t.Fatalf("ac + olay: %q", c)
	}
	zamanKomut(z, []string{"olay"})
	if dosyaVar(z.Olay) {
		t.Fatal("argümansız olay değiştirmeli")
	}
	if c := zamanKomut(z, []string{"olay", "belki"}); c != zamanKullanim {
		t.Fatal("olay belki")
	}
	zamanKomut(z, []string{"kapat"})
	if dosyaVar(z.Acik) {
		t.Fatal("kapat")
	}
	if c := zamanKomut(z, []string{"olay", "aç"}); !strings.Contains(c, "şu an kapalı") {
		t.Fatalf("time-lapse kapalıyken olay: %q", c)
	}
	os.WriteFile(z.Aralik, []byte("9999\n"), 0644)
	if zamanAralikOku(z.Aralik) != 60 || zamanAralikOku(z.Aralik+"-yok") != 60 {
		t.Fatal("geçersiz/yok -> 60")
	}
}

func TestZamanDurumMetni(t *testing.T) {
	z := geciciZaman(t)
	os.WriteFile(z.Aralik, []byte("3\n"), 0644)
	bayrak(z.Acik, true)
	bayrak(z.Olay, true)
	os.WriteFile(z.Durum, []byte(`{"surum":"kamera-kayit-v9","zaman_olay":true,"zaman_mb":812.5,"zaman_sinir_mb":4096,"zaman_dolu":false}`), 0644)
	m := zamanDurumMetni(z)
	for _, p := range []string{"Time-lapse: AÇIK", "her 3 sn", "Olay tespiti: AÇIK", "812 / 4096 MB", "en eski gün silinir"} {
		if !strings.Contains(m, p) {
			t.Fatalf("%q yok: %s", p, m)
		}
	}
	os.WriteFile(z.Durum, []byte(`{"zaman_mb":4200,"zaman_sinir_mb":4096,"zaman_dolu":true}`), 0644)
	if m := zamanDurumMetni(z); !strings.Contains(m, "BUGÜN DOLDU") {
		t.Fatal(m)
	}
	os.WriteFile(z.Durum, []byte(`{"surum":"kamera-kayit-v8","zaman":true}`), 0644)
	if m := zamanDurumMetni(z); strings.Contains(m, "MB") {
		t.Fatalf("eski kamera-kayit doluluk yazmamalı: %s", m)
	}
	if k := zamanKisa(z, false); k != "açık (her 3 sn, olay tespiti)" {
		t.Fatal(k)
	}
	bayrak(z.Acik, false)
	if k := zamanKisa(z, false); k != "kapalı" {
		t.Fatal(k)
	}
	if aralikMetni(300) != "5 dk" || aralikMetni(90) != "90 sn" {
		t.Fatal("aralikMetni")
	}
}

func TestEylemZaman(t *testing.T) {
	durum := []struct {
		d string
		u map[string]any
	}{
		{"baslat", map[string]any{"zaman": "baslat"}}, {"durdur", map[string]any{"zaman": "durdur"}},
		{"olay_ac", map[string]any{"zaman_olay": "ac"}}, {"Olay_Kapat", map[string]any{"zaman_olay": "kapat"}},
		{"3", map[string]any{"zaman_aralik": 3}}, {"60", map[string]any{"zaman_aralik": 60}}, {"1 dk", map[string]any{"zaman_aralik": 60}},
		{"0", nil}, {"9999", nil}, {"olay", nil}, {"", nil},
	}
	for _, x := range durum {
		if u := eylemUi(eylem{Ad: "zaman_atlamali", Deger: x.d}); !reflect.DeepEqual(u, x.u) {
			t.Fatalf("%q: %v (beklenen %v)", x.d, u, x.u)
		}
	}
	if !strings.Contains(eylemTalimat, "'olay_ac'") || strings.Contains(eylemTalimat, "dakikada bir kare biriktirir") {
		t.Fatal("istem")
	}
}

func TestTgMenuZaman(t *testing.T) {
	s := yeniSahteTg(t)
	if !tgMenuKomut(context.Background(), "J", "42", "⏱ Time-lapse", "") {
		t.Fatal("⏱ işlenmedi")
	}
	f := s.sonForm("sendMessage")
	g := f.Get("reply_markup")
	for _, p := range []string{`"k:/zaman ac"`, `"k:/zaman kapat"`, `"k:/zaman 1"`, `"k:/zaman 3"`, `"k:/zaman 60"`, `"k:/zaman olay ac"`,
		`"k:/zaman olay kapat"`} {
		if !strings.Contains(g, p) {
			t.Fatalf("%s yok: %s", p, g)
		}
	}
	if !strings.Contains(f.Get("text"), "Olay tespiti") || strings.Contains(f.Get("text"), "dakikada bir kare") {
		t.Fatal(f.Get("text"))
	}
	if tgMenuKomut(context.Background(), "J", "42", "/zaman 3", "") || tgMenuKomut(context.Background(), "J", "42", "/zaman olay ac", "") {
		t.Fatal("argümanlı /zaman menüde yutulmamalı")
	}
}

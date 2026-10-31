// heykole_test.go (23.10.2026, Claude; v35): /heykole, sesle kapatma, menü düğmeleri.
package main

import (
	"context"
	"path/filepath"
	"reflect"
	"strings"
	"testing"
)

func TestHeyKoleKomut(t *testing.T) {
	eski := heyKoleKapali
	heyKoleKapali = filepath.Join(t.TempDir(), "kapali")
	defer func() { heyKoleKapali = eski }()
	if !strings.Contains(heyKoleKomut(""), "açık") || heyKoleKisa() != "Hey Köle açık" {
		t.Fatal("başta açık")
	}
	if c := heyKoleKomut("kapat"); !dosyaVar(heyKoleKapali) || !strings.Contains(c, "kapatıldı") {
		t.Fatal(c)
	}
	if heyKoleKisa() != "Hey Köle kapalı" || !strings.Contains(heyKoleDurum(), "kapalı") {
		t.Fatal("kapalı durum")
	}
	heyKoleKomut("kapat") // ikinci kez: hata yok
	if c := heyKoleKomut("aç"); dosyaVar(heyKoleKapali) || !strings.Contains(c, "açıldı") {
		t.Fatal(c)
	}
	heyKoleKomut("ac")
	if c := heyKoleKomut("belki"); dosyaVar(heyKoleKapali) || !strings.Contains(c, "Kullanım") {
		t.Fatal("bilinmeyen argüman değiştirmemeli")
	}
}

func TestEylemHeyKole(t *testing.T) {
	if u := eylemUi(eylem{Ad: "hey_kole", Deger: "Kapat"}); !reflect.DeepEqual(u, map[string]any{"heykole": false}) {
		t.Fatal(u)
	}
	for _, d := range []string{"ac", "aç", "", "sessiz"} {
		if u := eylemUi(eylem{Ad: "hey_kole", Deger: d}); u != nil {
			t.Fatalf("%q: %v (sesle açma yok)", d, u)
		}
	}
	if eylemAdlari[len(eylemAdlari)-1] != "hey_kole" || !strings.Contains(eylemTalimat, "- hey_kole: 'kapat'") {
		t.Fatal("enum/istem")
	}
}

func TestTgMenuHeyKole(t *testing.T) {
	if !strings.HasSuffix(tgMenuBayrak, "telegram-menu-v50") { // v50: Gezi en üstte // yeniSahteTg yolu geçici dizine çevirmeden önce
		t.Fatal("menü bayrağı v50 değil:", tgMenuBayrak)
	}
	s := yeniSahteTg(t)
	ctx := context.Background()
	if !tgMenuKomut(ctx, "J", "42", "/heykole", "") {
		t.Fatal("argümansız /heykole menü göstermeli")
	}
	if g := s.sonForm("sendMessage").Get("reply_markup"); !strings.Contains(g, `"k:/heykole ac"`) || !strings.Contains(g, `"k:/heykole kapat"`) {
		t.Fatal(g)
	}
	tgMenuKomut(ctx, "J", "42", "🔊 Ses", "")
	if g := s.sonForm("sendMessage").Get("reply_markup"); !strings.Contains(g, `"k:/heykole kapat"`) || !strings.Contains(g, `"k:/sesac"`) {
		t.Fatal(g)
	}
	if tgMenuKomut(ctx, "J", "42", "/heykole kapat", "") {
		t.Fatal("argümanlı /heykole menüde yutulmamalı")
	}
	bulundu := false
	for _, k := range tgKomutlar {
		bulundu = bulundu || k[0] == "heykole"
	}
	if !bulundu {
		t.Fatal("sol menüde heykole yok")
	}
}

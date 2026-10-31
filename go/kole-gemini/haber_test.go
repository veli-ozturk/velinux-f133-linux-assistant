package main

import (
	"strings"
	"testing"
	"time"
	"unicode/utf8"
)

func TestHaberKumele(t *testing.T) {
	simdi := time.Date(2026, 10, 19, 12, 0, 0, 0, time.UTC)
	yeni := simdi.Add(-time.Hour)
	oge := []haberOge{
		{"Merkez Bankası politika faizini yüzde 40'ta sabit tuttu", "AA", katEkonomi, yeni},
		{"Merkez Bankası faizi yüzde 40'ta sabit bıraktı", "NTV", katEkonomi, yeni},
		{"Merkez Bankası faiz kararını açıkladı: politika faizi sabit", "TRT", katEkonomi, yeni},
		{"Yeni akıllı telefon modeli tanıtıldı", "ShiftDelete", katTeknoloji, yeni},
		{"Galatasaray maçında gol yağmuru", "NTV", katGundem, yeni},                            // spor -> atılır
		{"KPSS sonuçları ne zaman açıklanacak?", "AA", katGundem, yeni},                        // SEO -> atılır
		{"Eski bir haber başlığı burada duruyor", "AA", katGundem, simdi.Add(-30 * time.Hour)}, // eski -> atılır
		{"Meclis yeni yasa teklifini kabul etti", "AA", katGundem, time.Time{}},                // zamansız -> kalır
	}
	k := haberKumele(oge, simdi)
	if len(k) != 3 {
		for _, c := range k {
			t.Log(c.Baslik, c.Kaynaklar)
		}
		t.Fatalf("3 küme beklendi, %d", len(k))
	}
	if len(k[0].Kaynaklar) != 3 || k[0].Kat != katEkonomi {
		t.Fatalf("ilk küme 3 kaynaklı faiz haberi olmalı: %+v", k[0])
	}
	b := haberBulteniYaz(k, 3, 2, "")
	if !strings.Contains(b, "🔥 Öne çıkanlar") || !strings.Contains(b, "(AA, NTV, TRT)") ||
		!strings.Contains(b, "💻 Teknoloji:") || !strings.Contains(b, "🏛 Gündem ve siyaset:") {
		t.Fatal(b)
	}
	if strings.Count(b, "Merkez Bankası") != 1 {
		t.Fatal("öne çıkan haber kategoride tekrar etmemeli:\n" + b)
	}
	if s := haberBulteniYaz(k, 0, 8, katTeknoloji); strings.Contains(s, "Öne çıkanlar") || !strings.Contains(s, "telefon") ||
		strings.Contains(s, "Meclis") {
		t.Fatal(s)
	}
}

func TestHaberSEO(t *testing.T) {
	for _, b := range []string{
		"Son dakika deprem mi oldu? Az önce deprem nerede oldu? İstanbul, Ankara, İzmir ve il il AFAD son depremler 19 Ekim 2026.",
		"BARAJ DOLULUK ORANLARI 18 EKİM 2026! İstanbul, Ankara, İzmir, Bursa baraj doluluk oranları kaç oldu?",
		"Afrika'nın ilk olimpiyat etkinliği Dakar 2026 için geri sayım başladı",
	} {
		if !haberGurultuMu(b) {
			t.Fatal("elenmeli:", b)
		}
	}
	if haberGurultuMu("Cumhuriyet'le yaşıt Ankara Lisesi bir asrı aşan eğitim geleneğini sürdürüyor") ||
		haberGurultuMu("NATO zirvesi Brüksel'de başladı") {
		t.Fatal("normal başlık elenmemeli")
	}
	if got := sehirSuz([]string{"BARAJ DOLULUK ORANLARI ANKARA İZMİR BURSA KAÇ OLDU?", "Ankara'da yeni metro hattı açıldı"}, "Ankara", 3); len(got) != 1 {
		t.Fatal(got)
	}
}

func TestHaberBenzerFarkli(t *testing.T) {
	a := haberKelimeler("İstanbul'da şiddetli yağış hayatı olumsuz etkiledi")
	b := haberKelimeler("Ankara'da yeni metro hattı hizmete açıldı")
	if haberBenzer(a, b) {
		t.Fatal("farklı haberler birleşmemeli")
	}
}

func TestHaberKatCoz(t *testing.T) {
	for g, w := range map[string]string{"": "", "siyaset": katGundem, "Teknoloji": katTeknoloji, "dünya": katDunya, "ekonomi": katEkonomi} {
		if k, ok := haberKatCoz(g); !ok || k != w {
			t.Fatal(g, k, ok)
		}
	}
	if _, ok := haberKatCoz("magazin"); ok {
		t.Fatal("magazin kategorisi olmamalı")
	}
}

func TestHaberZaman(t *testing.T) {
	for _, s := range []string{"Sun, 19 Oct 2026 10:00:00 +0300", "Sun, 19 Oct 2026 07:00:00 GMT", "2026-10-19T10:00:00+03:00"} {
		if haberZaman(s).IsZero() {
			t.Fatal(s)
		}
	}
}

func TestYardimMetni(t *testing.T) {
	if n := utf8.RuneCountInString(yardimMetni); n > 4000 {
		t.Fatalf("Telegram sınırı: %d", n)
	}
	k := trKucuk(yardimMetni)
	if strings.Contains(k, "namaz") || strings.Contains(k, "ezan") {
		t.Fatal("yardımda namaz/ezan olmamalı")
	}
	for _, c := range []string{"/haber", "/sabah", "/sehir", "/foto", "/bekci", "/zaman", "/cerceve", "/durum", "/saglik"} {
		if !strings.Contains(yardimMetni, c) {
			t.Fatal(c)
		}
	}
}

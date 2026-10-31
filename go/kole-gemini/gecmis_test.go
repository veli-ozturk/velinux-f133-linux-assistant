package main

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestGecmisBosSoru(t *testing.T) {
	g := gecmis{Tur: []sonuc{
		{Soru: "", Cevap: "Tam anlayamadım efendim, tekrar eder misiniz?"}, // 21.10 cihazdaki hata durumu
		{Soru: "hava nasıl", Cevap: "Güneşli efendim."},
		{Soru: "  ", Cevap: "x"},
		{Soru: "saat kaç", Cevap: " "},
	}}
	ic := gecmisIcerik(g)
	if len(ic) != 2 || ic[0].Parts[0].Text != "hava nasıl" || ic[1].Role != "model" {
		t.Fatalf("yalnız dolu tur kalmalı: %+v", ic)
	}
	ist := istek{Contents: append(ic, icerik{Role: "user", Parts: []parca{{Text: "yeni soru"}}})}
	b, _ := json.Marshal(ist)
	if strings.Contains(string(b), `"parts":[{}]`) || strings.Contains(string(b), `[{}`) {
		t.Fatalf("boş parça gitmemeli: %s", b)
	}
	if gecmiseEkle(&g, sonuc{Soru: "", Cevap: "Tam anlayamadım efendim"}) || len(g.Tur) != 4 {
		t.Fatal("anlaşılmayan soru geçmişe eklenmemeli")
	}
	if !gecmiseEkle(&g, sonuc{Soru: "dolar kaç", Cevap: "41 lira efendim."}) || len(g.Tur) != 5 {
		t.Fatal("anlaşılan soru eklenmeli")
	}
}

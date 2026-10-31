// gecmis.go (21.10.2026, Claude; kole-gemini v27, OTONOM Sıra 8 madde 59 yan bulgusu): kısa bağlamdan istek içeriği.
// Hata (cihaz asistan.log 21.10 ~11:00): anlaşılamayan sesli soru ('soru' boş, cevap "Tam anlayamadım") geçmişe
// yazılıyordu; sonraki 10 dk (gecmisOmur) her istekte contents[0].parts[0] boş metinle ({}) gidip TÜM modeller
// HTTP 400 "required oneof field 'data' must have one initialized field" verdi -> Hey Köle "şu an bağlantı kuramıyorum".
// v27: boş soru ya da cevaplı tur isteğe konmaz (eski dosyada kalsa bile) ve geçmişe hiç yazılmaz.
package main

import "strings"

func gecmisIcerik(g gecmis) []icerik {
	var o []icerik
	for _, t := range g.Tur {
		s, c := strings.TrimSpace(t.Soru), strings.TrimSpace(t.Cevap)
		if s == "" || c == "" {
			continue
		}
		o = append(o, icerik{Role: "user", Parts: []parca{{Text: s}}}, icerik{Role: "model", Parts: []parca{{Text: c}}})
	}
	return o
}

// gecmiseEkle: yalnız anlaşılmış (sorusu ve cevabı dolu) tur eklenir
func gecmiseEkle(g *gecmis, s sonuc) bool {
	if strings.TrimSpace(s.Soru) == "" || strings.TrimSpace(s.Cevap) == "" {
		return false
	}
	g.Tur = append(g.Tur, sonuc{Soru: s.Soru, Cevap: s.Cevap})
	return true
}

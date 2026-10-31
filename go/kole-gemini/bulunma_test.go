package main

import "testing"

func TestBulunma(t *testing.T) {
	for ad, b := range map[string]string{"Ankara": "Ankara'da", "İzmir": "İzmir'de", "İstanbul": "İstanbul'da",
		"Bursa": "Bursa'da", "Antep": "Antep'te", "Erzurum": "Erzurum'da", "Rize": "Rize'de", "Muş": "Muş'ta"} {
		if g := bulunma(ad); g != b {
			t.Errorf("%s: %s, beklenen %s", ad, g, b)
		}
	}
}

package main

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestGunlerListe(t *testing.T) {
	h := havaVeri{Gun: 3}
	h.Max[0], h.Min[0], h.GunKod[0], h.Yagis[0] = 14.4, 2.6, 3, 10
	h.Max[1], h.Min[1], h.GunKod[1], h.Yagis[1] = 15.5, -0.5, 61, 80
	h.Max[2], h.Min[2], h.GunKod[2] = -3.5, -7.2, 71
	g := gunlerListe(h)
	if len(g) != 3 || g[0] != [4]int{3, 14, 3, 10} || g[1] != [4]int{61, 16, -1, 80} || g[2] != [4]int{71, -4, -7, 0} {
		t.Fatalf("gunler %v", g)
	}
	b, _ := json.Marshal(map[string]any{"gunler": g})
	if string(b) != `{"gunler":[[3,14,3,10],[61,16,-1,80],[71,-4,-7,0]]}` {
		t.Fatalf("json %s", b)
	}
	if len(gunlerListe(havaVeri{Gun: 9})) != 7 || len(gunlerListe(havaVeri{})) != 0 {
		t.Fatal("sinir")
	}
}

func TestVeriKirp(t *testing.T) {
	uzun := strings.Repeat("ş", 150) // 300 bayt; 8 baslik ~2400 bayt > sinir
	hb := []string{uzun, uzun, uzun, uzun, uzun, uzun, uzun, uzun}
	out := map[string]any{"zaman": 1, "hava": map[string]any{"kod": 0}, "gunler": [][4]int{{0, 1, 2, 3}}, "haber": hb}
	b := veriKirp(out, veriSinir)
	if len(b) > veriSinir {
		t.Fatalf("%d bayt", len(b))
	}
	var geri map[string]any
	if json.Unmarshal(b, &geri) != nil || geri["gunler"] == nil || geri["hava"] == nil {
		t.Fatalf("gunler/hava kalmali: %s", b[:80])
	}
	if n := len(geri["haber"].([]any)); n < 1 || n >= 8 {
		t.Fatalf("haber %d", n)
	}
	kucuk := map[string]any{"zaman": 1, "haber": []string{"a"}}
	if string(veriKirp(kucuk, veriSinir)) != `{"haber":["a"],"zaman":1}` {
		t.Fatal("kucuk veri degismemeli")
	}
	dev := map[string]any{"x": strings.Repeat("a", 3000), "gunler": [][4]int{{1, 2, 3, 4}}}
	veriKirp(dev, veriSinir) // haber yok: gunler atilir, sonra vazgecer (sonsuz dongu yok)
	if _, ok := dev["gunler"]; ok {
		t.Fatal("gunler atilmali")
	}
}

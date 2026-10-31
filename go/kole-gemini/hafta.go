// hafta.go (19.10.2026, Claude): v12 panel 0.45 haftalik serit verisi ve panel-veri.json boyut siniri.
package main

import (
	"encoding/json"
	"math"
)

// veriSinir: panel 0.44 ve oncesi panel-veri.json'u 2048 baytlik tamponla okur (sonu NUL); pay birakilir.
const veriSinir = 1900

// gunlerListe: bugunden baslayarak gelen gunler [kod, max, min, yagis] (tamsayi; panel 7 sutun cizer).
func gunlerListe(h havaVeri) [][4]int {
	n := h.Gun
	if n > len(h.Max) {
		n = len(h.Max)
	}
	g := make([][4]int, 0, n)
	for i := 0; i < n; i++ {
		g = append(g, [4]int{h.GunKod[i], int(math.Round(h.Max[i])), int(math.Round(h.Min[i])), h.Yagis[i]})
	}
	return g
}

// veriKirp: JSON siniri asarsa once son haber basliklarini, gerekirse "gunler"i atar (hava/doviz/namaz kalir).
func veriKirp(out map[string]any, sinir int) []byte {
	b, _ := json.Marshal(out)
	for len(b) > sinir {
		if h, ok := out["haber"].([]string); ok && len(h) > 0 {
			out["haber"] = h[:len(h)-1]
		} else if _, ok := out["gunler"]; ok {
			delete(out, "gunler")
		} else {
			break
		}
		b, _ = json.Marshal(out)
	}
	return b
}

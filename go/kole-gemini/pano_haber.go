// pano_haber.go (22.10.2026, Claude; kole-gemini v32, scripts/536): haber önbelleği. Telegram sürecindeki Pano turu
// (veriYaz, 10 dk) genel + siber başlıkları /tmp/haber-onbellek.json'a da yazar; Hey Köle / Asistan bunları bağlama alır,
// haber eylemi önce buradan okur. Ölçüm: haber eylemi her soruda bütün RSS akışlarını çekiyordu (BBC 12 sn zaman aşımı).
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"strings"
	"time"
)

var (
	haberOnbellekYol  = "/tmp/haber-onbellek.json"
	haberOnbellekOmur = 90 * time.Minute
)

type haberOnbellek struct {
	Zaman int64    `json:"zaman"`
	Genel []string `json:"genel"`
	Siber []string `json:"siber"`
}

func haberOnbellekYaz(genel, siber []string, simdi time.Time) {
	if len(genel) == 0 && len(siber) == 0 {
		return
	}
	if len(genel) > 8 {
		genel = genel[:8]
	}
	if len(siber) > 5 {
		siber = siber[:5]
	}
	b, _ := json.Marshal(haberOnbellek{simdi.Unix(), genel, siber})
	_ = dosyaYazAtomik(haberOnbellekYol, b, 0644)
}

// genel, siber, yaş; ok: dosya var, okunur ve haberOnbellekOmur'den taze
func haberOnbellekOku(simdi time.Time) ([]string, []string, time.Duration, bool) {
	b, err := os.ReadFile(haberOnbellekYol)
	if err != nil {
		return nil, nil, 0, false
	}
	var h haberOnbellek
	if json.Unmarshal(b, &h) != nil || h.Zaman == 0 {
		return nil, nil, 0, false
	}
	yas := simdi.Sub(time.Unix(h.Zaman, 0))
	if yas < -time.Minute || yas > haberOnbellekOmur || len(h.Genel)+len(h.Siber) == 0 {
		return nil, nil, 0, false
	}
	return h.Genel, h.Siber, yas, true
}

// basliklarCumle: en çok 3 başlık, sonlarına nokta ("...mi?." olmasın)
func basliklarCumle(onek string, h []string) string {
	if len(h) > 3 {
		h = h[:3]
	}
	o := make([]string, 0, len(h))
	for _, t := range h {
		t = strings.TrimSpace(t)
		if !strings.HasSuffix(t, "?") && !strings.HasSuffix(t, "!") && !strings.HasSuffix(t, ".") {
			t += "."
		}
		o = append(o, t)
	}
	return onek + strings.Join(o, " ")
}

// siberCumlesi: önbellekten, yoksa canlı (iki küçük akış)
func siberCumlesi(ctx context.Context) (string, error) {
	_, s, _, ok := haberOnbellekOku(time.Now())
	if !ok || len(s) == 0 {
		s = siberPano(ctx, 3)
	}
	if len(s) == 0 {
		return "", fmt.Errorf("siber haber yok")
	}
	return basliklarCumle("Siber güvenlikte öne çıkanlar: ", s), nil
}

// panoHaberBaglam: Hey Köle / Asistan istemine eklenen güncel başlıklar ("" = önbellek yok/bayat)
func panoHaberBaglam(simdi time.Time) string {
	g, s, yas, ok := haberOnbellekOku(simdi)
	if !ok {
		return ""
	}
	if len(g) > 6 {
		g = g[:6]
	}
	o := fmt.Sprintf(" Güncel haber başlıkları (%d dk önce toplandı; yalnız haber sorulursa kullan)", int(yas.Minutes()))
	if len(g) > 0 {
		o += " — genel: " + strings.Join(g, " | ")
	}
	if len(s) > 0 {
		o += " — siber güvenlik: " + strings.Join(s, " | ")
	}
	return o + "."
}

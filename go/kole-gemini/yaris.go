// yaris.go (20.10.2026, Claude): v20 paralel yedekli model isteği (scripts/394). Cihazda 20.10 21:42: gemini-3.8-flash
// 503 (yüksek talep), flash-lite-latest 11 sn'de yanıt vermedi, üçüncüye süre kalmadı -> "bağlantı kuramıyorum".
// Sağlıklı 3.8-flash bu cihazda ~7.9 sn sürdüğü için model başına kesme yerine: yavaş modelin YANINA sıradaki başlar.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

const (
	llmSesliYedek  = 7 * time.Second // v31: sağlıklı lite modeller ~6 sn (v20: 9 sn, 3.8-flash ~7.9 sn)
	llmMetinYedek  = 8 * time.Second
	llmYavasSoguma = 3 * time.Minute
	llmYukSoguma   = 15 * time.Minute // v31: 503 "high demand" dakikalarca sürüyor (22.10: 3 dk sonra yine 503)
)

const (
	yarisTamam = iota
	yarisZaman // genel süre doldu
	yarisAg    // ağ hatası (DNS/bağlantı): başka model denemek anlamsız
	yarisYok   // hepsi hata verdi
)

type llmDeneme struct {
	model  string
	kod    int
	baslik http.Header
	b      []byte
	y      yanit
	err    error
	ms     int64
}

func (d llmDeneme) basarili() bool {
	return d.err == nil && d.kod == 200 && d.y.Error == nil && len(d.y.Candidates) > 0
}

// istekten iki gövde: düşünme ayarlı ve ayarsız (bazı modeller thinkingConfig'i 400 ile reddeder)
func llmGovdeler(ist istek) (dusunceli, dusuncesiz []byte) {
	dusunceli, _ = json.Marshal(ist)
	gc := map[string]any{}
	for k, v := range ist.GenerationConfig {
		if k != "thinkingConfig" {
			gc[k] = v
		}
	}
	ist.GenerationConfig = gc
	dusuncesiz, _ = json.Marshal(ist)
	return
}

// başarısız denemeyi günlüğe yazar ve soğumaya koyar (yalnız ana iş parçacığından çağrılır: dosya yarışı yok)
func llmHataIsle(d llmDeneme, anahtar string) {
	if d.err != nil {
		switch {
		case errors.Is(d.err, context.Canceled):
		case errors.Is(d.err, context.DeadlineExceeded):
			fmt.Fprintf(os.Stderr, "%s: genel sure doldu (%d ms)\n", d.model, d.ms)
		default:
			fmt.Fprintf(os.Stderr, "%s: istek hatasi (%d ms): %s\n", d.model, d.ms, strings.ReplaceAll(d.err.Error(), anahtar, "***"))
		}
		return
	}
	msg := strings.ReplaceAll(string(d.b), anahtar, "***")
	if d.y.Error != nil {
		msg = d.y.Error.Message
	}
	if len(msg) > 160 {
		msg = msg[:160]
	}
	fmt.Fprintf(os.Stderr, "%s: HTTP %d (%d ms) %s\n", d.model, d.kod, d.ms, msg)
	switch {
	case d.kod == 429: // v17: kota hafızası
		sogumaKoyKota("llm:"+d.model, []byte(strings.ReplaceAll(string(d.b), anahtar, "***")), d.baslik.Get("Retry-After"))
	case d.kod == 404:
		sogumaKoy("llm:"+d.model, 24*time.Hour, "404")
	case d.kod >= 500: // v19: aşırı yük (503) / sunucu hatası; v31: 15 dk
		sogumaKoy("llm:"+d.model, llmYukSoguma, strconv.Itoa(d.kod))
	}
}

// modeller sırayla: çalışan istek `yedek` sürede bitmezse sıradaki YANINA başlar (en çok `azami` paralel); hızlı hata
// gelince sıradaki hemen başlar. İlk geçerli 200 kazanır, diğerleri iptal edilir.
func llmYaris(ctx context.Context, modeller []string, ist istek, anahtar string, yedek time.Duration, azami int) (yanit, string, int) {
	g1, g2 := llmGovdeler(ist)
	rctx, iptal := context.WithCancel(ctx)
	defer iptal()
	dl, ok := ctx.Deadline()
	if !ok {
		dl = time.Now().Add(time.Minute)
	}
	var liste []string
	for _, m := range modeller {
		if m = strings.TrimSpace(m); m != "" {
			liste = append(liste, m)
		}
	}
	ch := make(chan llmDeneme, len(liste))
	calisiyor := map[string]time.Time{}
	sira, agHatasi := 0, false
	baslat := func() string {
		if sira >= len(liste) || agHatasi || ctx.Err() != nil {
			return ""
		}
		m := liste[sira]
		sira++
		calisiyor[m] = time.Now()
		go func() {
			t0 := time.Now()
			url := llmURLTaban + m + ":generateContent"
			res, b, err := llmIstek(rctx, url, g1, anahtar, time.Until(dl)+time.Second)
			if err == nil && res.StatusCode == 400 && strings.Contains(strings.ToLower(string(b)), "think") {
				res, b, err = llmIstek(rctx, url, g2, anahtar, time.Until(dl)+time.Second) // düşünme ayarsız yeniden
			}
			d := llmDeneme{model: m, err: err, ms: time.Since(t0).Milliseconds()}
			if err == nil {
				d.kod, d.baslik, d.b = res.StatusCode, res.Header, b
				_ = json.Unmarshal(b, &d.y)
			}
			ch <- d
		}()
		return m
	}
	if baslat() == "" {
		return yanit{}, "", yarisYok
	}
	zam := time.NewTimer(yedek)
	defer zam.Stop()
	for len(calisiyor) > 0 {
		select {
		case d := <-ch:
			delete(calisiyor, d.model)
			if d.basarili() {
				fmt.Fprintf(os.Stderr, "%s: cevap %d ms\n", d.model, d.ms)
				sogumaKaldir("llm:" + d.model)
				for m, t := range calisiyor { // yedek süresini aşıp kaybeden: sonraki sorularda sona
					if time.Since(t) >= yedek {
						sogumaKoy("llm:"+m, llmYavasSoguma, "yavas")
					}
				}
				return d.y, d.model, yarisTamam
			}
			llmHataIsle(d, anahtar)
			if d.err != nil && !errors.Is(d.err, context.Canceled) && !errors.Is(d.err, context.DeadlineExceeded) {
				agHatasi = true
			}
			if len(calisiyor) < azami && baslat() != "" {
				zam.Reset(yedek)
			}
		case <-zam.C:
			if len(calisiyor) < azami {
				var yavas []string
				for m := range calisiyor {
					yavas = append(yavas, m)
				}
				if m := baslat(); m != "" {
					fmt.Fprintf(os.Stderr, "%s yanit vermedi (%d ms), yaninda %s basladi\n", strings.Join(yavas, ","), yedek.Milliseconds(), m)
				}
			}
			zam.Reset(yedek)
		case <-ctx.Done():
			return yanit{}, "", yarisZaman
		}
	}
	switch {
	case ctx.Err() != nil:
		return yanit{}, "", yarisZaman
	case agHatasi:
		return yanit{}, "", yarisAg
	}
	return yanit{}, "", yarisYok
}

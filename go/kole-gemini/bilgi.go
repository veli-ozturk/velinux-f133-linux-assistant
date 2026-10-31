// bilgi.go (15.10.2026, Claude): güncel bilgi cihazın KENDİSİNDEN, ücretsiz/anahtarsız açık servislerden (PC'siz).
// Google Arama (Gemini grounding) bu anahtarın ücretsiz katmanında kotasız (429) -> hava Open-Meteo, döviz open.er-api.
// kullanıcı (15.10): "çektiği verileri PC'ye bağlı çekmesin, internetten çeksin; konum Ankara".
//
//	kole-gemini --veri   -> /tmp/panel-veri.json (panel 0.32 Pano: hava + döviz; autorun 10 dk'da bir çalıştırır)
//
// Ev şehri /data/asistan/sehir ("Ankara;39.93;32.86"), yoksa Ankara.
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"strconv"
	"strings"
	"time"
)

type konum struct {
	Ad         string
	Enlem, Boy float64
}

var evVarsayilan = konum{"Ankara", 39.93, 32.86}

func evSehri(yol string) konum {
	b, err := os.ReadFile(yol)
	if err != nil {
		return evVarsayilan
	}
	p := strings.Split(strings.TrimSpace(string(b)), ";")
	if len(p) != 3 {
		return evVarsayilan
	}
	e, err1 := strconv.ParseFloat(p[1], 64)
	g, err2 := strconv.ParseFloat(p[2], 64)
	if err1 != nil || err2 != nil || p[0] == "" {
		return evVarsayilan
	}
	return konum{p[0], e, g}
}

func httpJSON(ctx context.Context, u string, v any) error {
	req, _ := http.NewRequestWithContext(ctx, http.MethodGet, u, nil)
	req.Header.Set("User-Agent", "VeLinux-KOLE/1")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return err
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		return fmt.Errorf("HTTP %d", res.StatusCode)
	}
	b, err := io.ReadAll(io.LimitReader(res.Body, 1<<20))
	if err != nil {
		return err
	}
	return json.Unmarshal(b, v)
}

func sehirBul(ctx context.Context, ad string) (konum, error) {
	var r struct {
		Results []struct {
			Name      string  `json:"name"`
			Latitude  float64 `json:"latitude"`
			Longitude float64 `json:"longitude"`
		} `json:"results"`
	}
	u := "https://geocoding-api.open-meteo.com/v1/search?count=1&language=tr&format=json&name=" + url.QueryEscape(ad)
	if err := httpJSON(ctx, u, &r); err != nil {
		return konum{}, err
	}
	if len(r.Results) == 0 {
		return konum{}, fmt.Errorf("şehir bulunamadı")
	}
	return konum{r.Results[0].Name, r.Results[0].Latitude, r.Results[0].Longitude}, nil
}

// WMO hava kodu -> Türkçe
func havaAdi(k int) string {
	switch {
	case k == 0:
		return "açık"
	case k <= 2:
		return "parçalı bulutlu"
	case k == 3:
		return "kapalı"
	case k == 45 || k == 48:
		return "sisli"
	case k >= 51 && k <= 57:
		return "çiseliyor"
	case k >= 61 && k <= 67:
		return "yağmurlu"
	case k >= 71 && k <= 77:
		return "karlı"
	case k >= 80 && k <= 82:
		return "sağanak yağışlı"
	case k == 85 || k == 86:
		return "kar sağanaklı"
	case k >= 95:
		return "gök gürültülü fırtınalı"
	}
	return "değişken"
}

type havaVeri struct {
	Sehir            string
	Sicaklik, Ruzgar float64
	Kod              int
	Max, Min         [7]float64 // v12: 7 gun (0 = bugun)
	Yagis            [7]int
	GunKod           [7]int
	Gun              int // gelen gun sayisi (<= 7)
}

func havaGetir(ctx context.Context, k konum) (havaVeri, error) {
	var r struct {
		Current struct {
			T    float64 `json:"temperature_2m"`
			Kod  int     `json:"weather_code"`
			Wind float64 `json:"wind_speed_10m"`
		} `json:"current"`
		Daily struct {
			Max []float64 `json:"temperature_2m_max"`
			Min []float64 `json:"temperature_2m_min"`
			Pp  []int     `json:"precipitation_probability_max"`
			Kod []int     `json:"weather_code"`
		} `json:"daily"`
	}
	u := fmt.Sprintf("https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"+
		"&current=temperature_2m,weather_code,wind_speed_10m"+
		"&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,weather_code"+
		"&timezone=auto&forecast_days=7", k.Enlem, k.Boy)
	if err := httpJSON(ctx, u, &r); err != nil {
		return havaVeri{}, err
	}
	h := havaVeri{Sehir: k.Ad, Sicaklik: r.Current.T, Kod: r.Current.Kod, Ruzgar: r.Current.Wind}
	for i := 0; i < len(h.Max); i++ {
		if i < len(r.Daily.Max) && i < len(r.Daily.Min) {
			h.Max[i], h.Min[i] = r.Daily.Max[i], r.Daily.Min[i]
			if i < len(r.Daily.Kod) {
				h.Gun = i + 1
			}
		}
		if i < len(r.Daily.Pp) {
			h.Yagis[i] = r.Daily.Pp[i]
		}
		if i < len(r.Daily.Kod) {
			h.GunKod[i] = r.Daily.Kod[i]
		}
	}
	return h, nil
}

// bulunma eki: son ünlüye göre da/de, sert ünsüzden sonra ta/te ("Ankara'da", "İzmir'de", "Antep'te")
func bulunma(ad string) string {
	r := []rune(strings.TrimSpace(ad))
	kalin := true
	for i := len(r) - 1; i >= 0; i-- {
		if strings.ContainsRune("aıouAIOU", r[i]) {
			break
		}
		if strings.ContainsRune("eiöüEİÖÜ", r[i]) {
			kalin = false
			break
		}
	}
	sert := len(r) > 0 && strings.ContainsRune("çfhkpsştÇFHKPSŞT", r[len(r)-1])
	ek := map[[2]bool]string{{true, false}: "da", {false, false}: "de", {true, true}: "ta", {false, true}: "te"}[[2]bool{kalin, sert}]
	return string(r) + "'" + ek
}

// "Ankara", "İzmir yarın", "" (ev), "yarin"
func havaCumlesi(ctx context.Context, deger, evYol string) (string, error) {
	d := strings.TrimSpace(deger)
	yarin := false
	for _, s := range []string{"yarın", "yarin", "Yarın", "Yarin"} {
		if strings.Contains(d, s) {
			yarin = true
			d = strings.TrimSpace(strings.ReplaceAll(d, s, ""))
		}
	}
	k := evSehri(evYol)
	if d != "" && !strings.EqualFold(d, k.Ad) {
		var err error
		if k, err = sehirBul(ctx, d); err != nil {
			return "", err
		}
	}
	h, err := havaGetir(ctx, k)
	if err != nil {
		return "", err
	}
	if yarin {
		return fmt.Sprintf("Yarın %s %s bekleniyor, en yüksek %.0f, en düşük %.0f derece, yağış ihtimali yüzde %d",
			bulunma(h.Sehir), havaAdi(h.GunKod[1]), h.Max[1], h.Min[1], h.Yagis[1]), nil
	}
	return fmt.Sprintf("%s şu an %.0f derece ve %s. Bugün en yüksek %.0f, en düşük %.0f derece, yağış ihtimali yüzde %d",
		bulunma(h.Sehir), h.Sicaklik, havaAdi(h.Kod), h.Max[0], h.Min[0], h.Yagis[0]), nil
}

func kurlar(ctx context.Context) (map[string]float64, error) {
	var r struct {
		Rates map[string]float64 `json:"rates"`
	}
	if err := httpJSON(ctx, "https://open.er-api.com/v6/latest/USD", &r); err != nil {
		return nil, err
	}
	try := r.Rates["TRY"]
	if try <= 0 {
		return nil, fmt.Errorf("TRY yok")
	}
	out := map[string]float64{"USD": try}
	for _, c := range []string{"EUR", "GBP"} {
		if v := r.Rates[c]; v > 0 {
			out[c] = try / v
		}
	}
	return out, nil
}

func tl(v float64) string { return strings.Replace(fmt.Sprintf("%.2f", v), ".", ",", 1) }

func dovizCumlesi(ctx context.Context, deger string) (string, error) {
	k, err := kurlar(ctx)
	if err != nil {
		return "", err
	}
	d := strings.ToLower(deger)
	ad := map[string]string{"USD": "dolar", "EUR": "euro", "GBP": "sterlin"}
	var secim []string
	for kod, isim := range ad {
		if strings.Contains(d, strings.ToLower(kod)) || strings.Contains(d, isim) {
			secim = append(secim, kod)
		}
	}
	if len(secim) == 0 {
		secim = []string{"USD", "EUR"}
	}
	var p []string
	for _, kod := range []string{"USD", "EUR", "GBP"} {
		for _, s := range secim {
			if s == kod {
				p = append(p, fmt.Sprintf("1 %s %s lira", ad[kod], tl(k[kod])))
			}
		}
	}
	return strings.Join(p, ", "), nil
}

// karaktere göre kalıp (hepsinde "efendim")
func karakterCumle(k int, govde string) string {
	switch k {
	case 1:
		return "Arz ederim efendim: " + govde + "."
	case 2:
		return "Bip bop! " + govde + " efendim!"
	}
	return govde + " efendim."
}

// --veri: Pano için hava + döviz (PC'siz)
func veriYaz(yol, evYol string) error {
	ctx, iptal := context.WithTimeout(context.Background(), 25*time.Second)
	defer iptal()
	k := evSehri(evYol)
	out := map[string]any{"zaman": time.Now().Unix(), "sehir": k.Ad}
	if h, err := havaGetir(ctx, k); err == nil {
		out["hava"] = map[string]any{"sicaklik": h.Sicaklik, "durum": havaAdi(h.Kod), "kod": h.Kod, "ruzgar": h.Ruzgar,
			"max": h.Max[0], "min": h.Min[0], "yagis": h.Yagis[0],
			"yarin": map[string]any{"durum": havaAdi(h.GunKod[1]), "kod": h.GunKod[1], "max": h.Max[1], "min": h.Min[1], "yagis": h.Yagis[1]}}
		out["gunler"] = gunlerListe(h) // v12 haftalik serit
	} else {
		fmt.Fprintln(os.Stderr, "hava:", err)
	}
	if kr, err := kurlar(ctx); err == nil {
		out["doviz"] = kr
	} else {
		fmt.Fprintln(os.Stderr, "doviz:", err)
	}
	if v, err := namazGetir(ctx, k); err == nil {
		out["namaz"] = v
	} else {
		fmt.Fprintln(os.Stderr, "namaz:", err)
	}
	h, err := haberGetir(ctx, 8)
	if err != nil {
		fmt.Fprintln(os.Stderr, "haber:", err)
	}
	sb := siberPano(ctx, 5)                              // v32: önbelleğe 5, Pano'ya yine 2
	if hb := panoHaberBirlestir(h, sb, 8); len(hb) > 0 { // v29: son 2'si siber güvenlik
		if len(sb) > 2 {
			hb = panoHaberBirlestir(h, sb[:2], 8)
		}
		out["haber"] = hb
	}
	haberOnbellekYaz(h, sb, time.Now()) // v32
	if out["hava"] == nil && out["doviz"] == nil {
		return fmt.Errorf("veri alinamadi")
	}
	b := veriKirp(out, veriSinir)
	return dosyaYazAtomik(yol, b, 0644) // v28: Telegram süreci ve --veri aynı anda yazabilir
}

// sabah.go v10 (19.10.2026, Claude): namaz/ezan satırı KALDIRILDI (kullanıcı); haberler haber.go kategorili bülteni;
// yerel kaynaklarda Hürriyet yerine AA güncel.
// sabah.go (15.10.2026, Claude): kole-gemini v9 sabah özeti (kullanıcı: "önemli haberleri ve hava durumuna göre nasıl
// giyinmem gerektiğini sabah 8'de atsın; başka şehirdeysem şehri değiştirebileyim; bulunduğum şehirde önemli bir durum
// varsa onu da söylesin").
//   - günlük hava ayrıntısı (Open-Meteo): hissedilen en yüksek/düşük, yağış mm ve olasılık, rüzgar/hamle, UV, hava kodu
//   - giyim önerisi: KURAL TABANLI (Gemini kotasına bağlı değil)
//   - önemli durumlar: fırtına, kuvvetli rüzgar/yağış, kar/buzlanma, aşırı sıcak/soğuk + başlığında şehir adı geçen
//     yerel haberler (NTV, Hürriyet, CNN Türk, TRT; RSS/Atom)
//   - genel önemli haberler (BBC Türkçe, mevcut haberGetir)
//   - /sehir <ad> (Open-Meteo coğrafi kodlama) -> /data/asistan/sehir "Ad;enlem;boylam" (Pano, hava, namaz da kullanır)
package main

import (
	"context"
	"encoding/xml"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
)

var yerelKaynaklar = []string{
	"https://www.ntv.com.tr/turkiye.rss",
	"https://www.aa.com.tr/tr/rss/default?cat=guncel",
	"https://www.cnnturk.com/feed/rss/turkiye/news",
	"https://www.trthaber.com/manset_articles.rss",
}

type gunHava struct {
	Max, Min, HisMax, HisMin, YagisMM, Ruzgar, Hamle, UV float64
	YagisOlas, Kod                                       int
}

func gunHavaGetir(ctx context.Context, k konum) (gunHava, error) {
	var r struct {
		Daily struct {
			Max    []float64 `json:"temperature_2m_max"`
			Min    []float64 `json:"temperature_2m_min"`
			HisMax []float64 `json:"apparent_temperature_max"`
			HisMin []float64 `json:"apparent_temperature_min"`
			Yagis  []float64 `json:"precipitation_sum"`
			Olas   []int     `json:"precipitation_probability_max"`
			Ruzgar []float64 `json:"wind_speed_10m_max"`
			Hamle  []float64 `json:"wind_gusts_10m_max"`
			UV     []float64 `json:"uv_index_max"`
			Kod    []int     `json:"weather_code"`
		} `json:"daily"`
	}
	u := fmt.Sprintf("https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&daily=temperature_2m_max,"+
		"temperature_2m_min,apparent_temperature_max,apparent_temperature_min,precipitation_sum,precipitation_probability_max,"+
		"wind_speed_10m_max,wind_gusts_10m_max,uv_index_max,weather_code&timezone=auto&forecast_days=1", k.Enlem, k.Boy)
	if err := httpJSON(ctx, u, &r); err != nil {
		return gunHava{}, err
	}
	d := r.Daily
	if len(d.Max) == 0 || len(d.Min) == 0 {
		return gunHava{}, fmt.Errorf("günlük hava yok")
	}
	g := gunHava{Max: d.Max[0], Min: d.Min[0], HisMax: d.Max[0], HisMin: d.Min[0]}
	if len(d.HisMax) > 0 && len(d.HisMin) > 0 {
		g.HisMax, g.HisMin = d.HisMax[0], d.HisMin[0]
	}
	if len(d.Yagis) > 0 {
		g.YagisMM = d.Yagis[0]
	}
	if len(d.Olas) > 0 {
		g.YagisOlas = d.Olas[0]
	}
	if len(d.Ruzgar) > 0 {
		g.Ruzgar = d.Ruzgar[0]
	}
	if len(d.Hamle) > 0 {
		g.Hamle = d.Hamle[0]
	}
	if len(d.UV) > 0 {
		g.UV = d.UV[0]
	}
	if len(d.Kod) > 0 {
		g.Kod = d.Kod[0]
	}
	return g, nil
}

func karli(k int) bool { return (k >= 71 && k <= 77) || k == 85 || k == 86 }

// giyimOnerisi: hissedilen sıcaklık bantları + yağış/rüzgar/kar/UV eklemeleri
func giyimOnerisi(g gunHava) string {
	var p []string
	switch t := g.HisMax; {
	case t >= 30:
		p = append(p, "ince, açık renkli, pamuklu kıyafetler; şapka ve güneş gözlüğü")
	case t >= 24:
		p = append(p, "tişört ve ince pantolon ya da şort")
	case t >= 17:
		p = append(p, "tişört ya da ince uzun kollu, yanınıza hafif bir ceket")
	case t >= 10:
		p = append(p, "kazak ya da sweatshirt ve bir mont")
	case t >= 3:
		p = append(p, "kalın mont, kazak ve uzun pantolon")
	default:
		p = append(p, "kalın kaban, atkı, bere, eldiven; mümkünse termal içlik")
	}
	if g.HisMax-g.HisMin >= 10 {
		p = append(p, fmt.Sprintf("sabah-akşam serin (hissedilen %.0f°), katmanlı giyinin", g.HisMin))
	}
	if karli(g.Kod) {
		p = append(p, "kaymaz tabanlı bot")
	}
	if g.YagisMM >= 10 {
		p = append(p, "su geçirmez mont/ayakkabı ve şemsiye")
	} else if g.YagisOlas >= 50 || g.YagisMM >= 2 {
		p = append(p, "şemsiye alın")
	}
	if g.Hamle >= 60 || g.Ruzgar >= 40 {
		p = append(p, "rüzgarlık (şemsiye zorlanabilir)")
	}
	if g.UV >= 7 {
		p = append(p, "güneş kremi")
	}
	return strings.Join(p, "; ")
}

// havaUyarilari: şehirdeki önemli hava durumları
func havaUyarilari(g gunHava) []string {
	var u []string
	if g.Kod >= 95 {
		u = append(u, "gök gürültülü fırtına bekleniyor")
	}
	if g.Hamle >= 70 {
		u = append(u, fmt.Sprintf("kuvvetli rüzgar (hamle %.0f km/sa)", g.Hamle))
	}
	if g.YagisMM >= 20 {
		u = append(u, fmt.Sprintf("kuvvetli yağış (%.0f mm), sel ve su baskınına dikkat", g.YagisMM))
	}
	if karli(g.Kod) {
		u = append(u, "kar yağışı bekleniyor")
	}
	if g.Min <= 0 && (g.YagisMM > 0 || g.YagisOlas >= 40) {
		u = append(u, "buzlanma ve don riski")
	}
	if g.HisMax >= 37 {
		u = append(u, fmt.Sprintf("aşırı sıcak (hissedilen %.0f°)", g.HisMax))
	}
	if g.HisMin <= -10 {
		u = append(u, fmt.Sprintf("aşırı soğuk (hissedilen %.0f°)", g.HisMin))
	}
	return u
}

// trKucuk: Türkçe küçük harf (I->ı, İ->i)
func trKucuk(s string) string {
	return strings.ToLower(strings.NewReplacer("I", "ı", "İ", "i").Replace(s))
}

// rssBasliklar: RSS (channel>item>title) ya da Atom (entry>title)
func rssBasliklar(ctx context.Context, u string, n int) ([]string, error) {
	req, _ := http.NewRequestWithContext(ctx, http.MethodGet, u, nil)
	req.Header.Set("User-Agent", "VeLinux-KOLE/1")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		return nil, fmt.Errorf("HTTP %d", res.StatusCode)
	}
	var r struct {
		Items []struct {
			Title string `xml:"title"`
		} `xml:"channel>item"`
		Entries []struct {
			Title string `xml:"title"`
		} `xml:"entry"`
	}
	if err := xml.NewDecoder(io.LimitReader(res.Body, 3<<20)).Decode(&r); err != nil {
		return nil, err
	}
	var o []string
	for _, it := range r.Items {
		o = append(o, it.Title)
	}
	for _, it := range r.Entries {
		o = append(o, it.Title)
	}
	var t []string
	for _, s := range o {
		s = strings.Join(strings.Fields(s), " ")
		if s != "" {
			t = append(t, s)
		}
		if len(t) == n {
			break
		}
	}
	return t, nil
}

// sehirSuz: başlığında şehir adı geçenler (tekrarsız, en çok n)
func sehirSuz(basliklar []string, sehir string, n int) []string {
	a := trKucuk(strings.TrimSpace(sehir))
	if a == "" {
		return nil
	}
	gor := map[string]bool{}
	var o []string
	for _, b := range basliklar {
		k := trKucuk(b)
		if !strings.Contains(k, a) || gor[k] || haberGurultuMu(b) {
			continue
		}
		gor[k] = true
		o = append(o, b)
		if len(o) == n {
			break
		}
	}
	return o
}

func sehirHaberleri(ctx context.Context, sehir string, n int) []string {
	var hepsi []string
	for _, u := range yerelKaynaklar {
		c, iptal := context.WithTimeout(ctx, 8*time.Second)
		b, err := rssBasliklar(c, u, 100)
		iptal()
		if err == nil {
			hepsi = append(hepsi, b...)
		}
	}
	return sehirSuz(hepsi, sehir, n)
}

func noktala(t string) string {
	if strings.HasSuffix(t, "?") || strings.HasSuffix(t, "!") || strings.HasSuffix(t, ".") {
		return t
	}
	return t + "."
}

// sabahOzeti (v9): saglik.go'daki taslağın yerine
func sabahOzeti(ctx context.Context, sehirYol string) string {
	k := evSehri(sehirYol)
	p := []string{"☀️ Günaydın efendim. " + trTarih(time.Now().In(trZaman)) + "."}
	if s, err := havaCumlesi(ctx, "", sehirYol); err == nil {
		p = append(p, "📍 "+s+".")
	}
	var onemli []string
	if g, err := gunHavaGetir(ctx, k); err == nil {
		p = append(p, "👕 Giyim: "+giyimOnerisi(g)+".")
		onemli = append(onemli, havaUyarilari(g)...)
	}
	for _, h := range sehirHaberleri(ctx, k.Ad, 3) {
		onemli = append(onemli, noktala(h))
	}
	if len(onemli) > 0 {
		p = append(p, "⚠️ "+k.Ad+" için önemli:")
		for _, o := range onemli {
			p = append(p, "  • "+o)
		}
	}
	if h, err := haberBulteni(ctx, 3, 2, ""); err == nil {
		p = append(p, "📰 Haberler:", h)
	}
	if s, err := dovizCumlesi(ctx, ""); err == nil {
		p = append(p, "💱 "+s+".")
	}
	p = append(p, "📝 "+hatirlaticiListesi()+".")
	p = append(p, kisaSaglik())
	return strings.Join(p, "\n")
}

// kisaSaglik: sabah özetinde tek satır; sorun varsa yalnız sorunlular
func kisaSaglik() string {
	hz := hizmetTara()
	var yok []string
	for _, h := range hizmetler {
		if hz[h.anahtar] == "" && !(h.anahtar == "asistan" && asistanKapali()) {
			yok = append(yok, h.ad)
		}
	}
	if len(yok) == 0 {
		return fmt.Sprintf("🟢 Cihaz: tüm hizmetler çalışıyor, boş bellek %d MB.", memAvailableKB()/1024)
	}
	return "🔴 Cihaz: çalışmayan hizmet: " + strings.Join(yok, ", ") + "."
}

// sehirKomutu: "/sehir" (göster), "/sehir İzmir" (değiştir), "/sehir sifirla" (Ankara)
func sehirKomutu(ctx context.Context, arg, sehirYol string) string {
	arg = strings.TrimSpace(arg)
	switch trKucuk(arg) {
	case "":
		k := evSehri(sehirYol)
		return fmt.Sprintf("Şu an %s için bilgi veriyorum efendim. Değiştirmek için: /sehir İzmir", k.Ad)
	case "sifirla", "sıfırla", "ev":
		_ = os.Remove(sehirYol)
		return "Konum varsayılana (" + evVarsayilan.Ad + ") döndü efendim."
	}
	k, err := sehirBul(ctx, arg)
	if err != nil {
		return "Bu şehri bulamadım efendim: " + arg
	}
	if err := dosyaYazAtomik(sehirYol, []byte(fmt.Sprintf("%s;%.4f;%.4f\n", k.Ad, k.Enlem, k.Boy)), 0644); err != nil { // v43 (141)
		return "Konumu kaydedemedim efendim."
	}
	return fmt.Sprintf("Konumunuz %s olarak ayarlandı efendim; hava, giyim önerisi, yerel uyarılar ve sabah özeti buna göre.", k.Ad)
}

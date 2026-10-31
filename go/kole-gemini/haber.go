// haber.go (19.10.2026, Claude): kole-gemini v10 kategorili haber. kullanıcı: "haberler daha mantıklı ve düzgün gelsin,
// teknoloji, siyaset, önemliler".
//   - Kaynaklar: AA, TRT Haber, NTV, BBC Türkçe (gündem/siyaset, ekonomi, dünya); teknoloji için ayrıca ShiftDelete,
//     Webtekno, DonanımHaber. Hürriyet çıkarıldı (SEO başlıkları, 1 MB'a yakın akış).
//   - Akışlar eşzamanlı çekilir (akış başına 8 sn, en çok 1 MB); son 24 saat dışındaki haberler atılır.
//   - "Önemli": aynı olayı kaç AYRI kaynağın verdiği (başlık kelime benzerliği) + birkaç anahtar sözcük.
//   - Magazin/spor/burç/"nasıl yapılır" gibi gürültü başlıkları atılır; benzer başlıklar tek habere indirilir.
package main

import (
	"context"
	"encoding/xml"
	"fmt"
	"io"
	"net/http"
	"sort"
	"strings"
	"sync"
	"time"
	"unicode"
)

type haberKaynak struct {
	Ad, Kat, URL string
}

const (
	katGundem    = "gundem"
	katEkonomi   = "ekonomi"
	katTeknoloji = "teknoloji"
	katDunya     = "dunya"
)

var haberKategoriAd = map[string]string{
	katGundem: "🏛 Gündem ve siyaset", katEkonomi: "💹 Ekonomi", katTeknoloji: "💻 Teknoloji", katDunya: "🌍 Dünya",
	katSiber: "🛡 Siber güvenlik", // v29
}

var haberKategoriSira = []string{katGundem, katEkonomi, katTeknoloji, katSiber, katDunya} // v29: siber

var haberKaynaklari = []haberKaynak{
	{"AA", katGundem, "https://www.aa.com.tr/tr/rss/default?cat=politika"},
	{"AA", katGundem, "https://www.aa.com.tr/tr/rss/default?cat=guncel"},
	{"TRT", katGundem, "https://www.trthaber.com/gundem_articles.rss"},
	{"NTV", katGundem, "https://www.ntv.com.tr/turkiye.rss"},
	{"BBC", katGundem, "https://feeds.bbci.co.uk/turkce/rss.xml"},
	{"AA", katEkonomi, "https://www.aa.com.tr/tr/rss/default?cat=ekonomi"},
	{"TRT", katEkonomi, "https://www.trthaber.com/ekonomi_articles.rss"},
	{"NTV", katEkonomi, "https://www.ntv.com.tr/ekonomi.rss"},
	{"AA", katTeknoloji, "https://www.aa.com.tr/tr/rss/default?cat=bilim-teknoloji"},
	{"TRT", katTeknoloji, "https://www.trthaber.com/bilim_teknoloji_articles.rss"},
	{"NTV", katTeknoloji, "https://www.ntv.com.tr/teknoloji.rss"},
	{"ShiftDelete", katTeknoloji, "https://shiftdelete.net/feed"},
	{"Webtekno", katTeknoloji, "https://www.webtekno.com/rss.xml"},
	{"DonanımHaber", katTeknoloji, "https://www.donanimhaber.com/rss/tum/"},
	{"AA", katDunya, "https://www.aa.com.tr/tr/rss/default?cat=dunya"},
	{"TRT", katDunya, "https://www.trthaber.com/dunya_articles.rss"},
	{"NTV", katDunya, "https://www.ntv.com.tr/dunya.rss"},
	{"ShiftDelete", katSiber, "https://shiftdelete.net/guvenlik/feed"}, // v29 (haber_siber.go)
	{"BleepingComputer", katSiber, "https://www.bleepingcomputer.com/feed/"},
	{"The Hacker News", katSiber, "https://feeds.feedburner.com/TheHackersNews"},
	{"SecurityWeek", katSiber, "https://www.securityweek.com/feed/"},
	{"Dark Reading", katSiber, "https://www.darkreading.com/rss.xml"},
}

type haberOge struct {
	Baslik, Kaynak, Kat string
	Zaman               time.Time // sıfır = bilinmiyor
}

type haberKume struct {
	haberOge
	Kaynaklar map[string]bool
	Puan      float64
	kelimeler map[string]bool
}

// gürültü: magazin, spor, burç, rehber/SEO, reklam
var haberGurultu = []string{"burç", "magazin", "ünlü oyuncu", "dizi", "fragman", "maç", "gol", "transfer", "süper lig",
	"olimpiyat", "şampiyon", "fenerbahçe", "galatasaray", "beşiktaş", "trabzonspor", "teknik direktör", "nasıl yapılır", "nedir?", "ne zaman açıklanacak",
	"sonuçları", "sorgulama", "canlı izle", "hangi kanalda", "indirim", "kampanya", "kupon", "çekiliş", "tarifi",
	"rüyada", "mi oldu", "kaç oldu", "il il", "doluluk oran", "nöbetçi eczane", "hava durumu", "namaz vakti", "ezan", "fotoğraf galerisi", "video haber"}

// önem artırıcılar (küçük ağırlık; asıl ölçü kaynak sayısı)
var haberOnemli = []string{"son dakika", "deprem", "selde", "sel felaketi", "patlama", "saldırı", "ateşkes", "savaş", "seçim", "meclis",
	"cumhurbaşkanı", "bakan", "kanun", "yasa", "anayasa", "merkez bankası", "faiz", "enflasyon", "asgari ücret", "vergi",
	"zam geldi", "dolar kuru", "borsa", "yapay zeka", "siber", "veri ihlali", "uydu", "nato", "bm ", "ab "}

func haberGurultuMu(b string) bool {
	if haberSEO(b) {
		return true
	}
	k := trKucuk(b)
	for _, g := range haberGurultu {
		if strings.Contains(k, g) {
			return true
		}
	}
	return false
}

// haberSEO: arama motoru başlığı kalıpları: çok soru işareti, BÜYÜK HARF ağırlıklı ya da çok uzun
func haberSEO(b string) bool {
	if strings.Count(b, "?") >= 2 || len([]rune(b)) > 150 {
		return true
	}
	buyuk, harf := 0, 0
	for _, r := range b {
		if unicode.IsLetter(r) {
			harf++
			if unicode.IsUpper(r) {
				buyuk++
			}
		}
	}
	return harf >= 12 && buyuk*10 >= harf*6
}

var haberDurak = map[string]bool{"için": true, "ile": true, "olan": true, "oldu": true, "dedi": true, "açıkladı": true,
	"yeni": true, "bir": true, "daha": true, "gibi": true, "kadar": true, "sonra": true, "önce": true, "olarak": true,
	"etti": true, "eden": true, "üzere": true, "değil": true, "tüm": true, "şunu": true, "bunu": true}

// haberKelimeler: karşılaştırma için kök benzeri (ilk 5 harf) kelime kümesi
func haberKelimeler(b string) map[string]bool {
	m := map[string]bool{}
	f := strings.FieldsFunc(trKucuk(b), func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9' || strings.ContainsRune("çğıöşüâî", r))
	})
	for _, w := range f {
		if len([]rune(w)) < 4 || haberDurak[w] {
			continue
		}
		r := []rune(w)
		if len(r) > 5 {
			r = r[:5]
		}
		m[string(r)] = true
	}
	return m
}

func haberBenzer(a, b map[string]bool) bool {
	if len(a) == 0 || len(b) == 0 {
		return false
	}
	ortak := 0
	for w := range a {
		if b[w] {
			ortak++
		}
	}
	kucuk := len(a)
	if len(b) < kucuk {
		kucuk = len(b)
	}
	return ortak >= 3 || (kucuk <= 4 && ortak >= 2 && float64(ortak)/float64(kucuk) >= 0.6)
}

var haberZamanBicim = []string{time.RFC1123Z, time.RFC1123, "Mon, 2 Jan 2006 15:04:05 -0700", "Mon, 2 Jan 2006 15:04:05 MST",
	time.RFC3339, "2006-01-02T15:04:05Z07:00", "2006-01-02 15:04:05"}

func haberZaman(s string) time.Time {
	s = strings.TrimSpace(s)
	for _, f := range haberZamanBicim {
		if t, err := time.Parse(f, s); err == nil {
			return t
		}
	}
	return time.Time{}
}

// haberAkis: tek akıştaki başlık + zaman (RSS item ya da Atom entry)
func haberAkis(ctx context.Context, k haberKaynak) ([]haberOge, error) {
	req, _ := http.NewRequestWithContext(ctx, http.MethodGet, k.URL, nil)
	req.Header.Set("User-Agent", "VeLinux-KOLE/1")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		return nil, fmt.Errorf("%s HTTP %d", k.Ad, res.StatusCode)
	}
	var r struct {
		Items []struct {
			Title string `xml:"title"`
			Pub   string `xml:"pubDate"`
		} `xml:"channel>item"`
		Entries []struct {
			Title   string `xml:"title"`
			Updated string `xml:"updated"`
		} `xml:"entry"`
	}
	if err := xml.NewDecoder(io.LimitReader(res.Body, 1<<20)).Decode(&r); err != nil && len(r.Items)+len(r.Entries) == 0 {
		return nil, err
	}
	var o []haberOge
	ekle := func(t, z string) {
		t = strings.Join(strings.Fields(t), " ")
		if t != "" && len(o) < 40 {
			o = append(o, haberOge{Baslik: t, Kaynak: k.Ad, Kat: k.Kat, Zaman: haberZaman(z)})
		}
	}
	for _, it := range r.Items {
		ekle(it.Title, it.Pub)
	}
	for _, it := range r.Entries {
		ekle(it.Title, it.Updated)
	}
	return o, nil
}

// haberTopla: tüm akışlar eşzamanlı; kategori filtresi "" = hepsi
func haberTopla(ctx context.Context, kat string) []haberOge {
	var (
		mu  sync.Mutex
		wg  sync.WaitGroup
		hep []haberOge
	)
	for _, k := range haberKaynaklari {
		if kat != "" && k.Kat != kat {
			continue
		}
		wg.Add(1)
		go func(k haberKaynak) {
			defer wg.Done()
			c, iptal := context.WithTimeout(ctx, 8*time.Second)
			defer iptal()
			o, err := haberAkis(c, k)
			if err != nil {
				return
			}
			mu.Lock()
			hep = append(hep, o...)
			mu.Unlock()
		}(k)
	}
	wg.Wait()
	return hep
}

// haberKumele: gürültü/eski at, benzerleri birleştir, puanla (saf işlev; test edilir)
func haberKumele(oge []haberOge, simdi time.Time) []*haberKume {
	var kum []*haberKume
	for _, h := range oge {
		if haberGurultuMu(h.Baslik) {
			continue
		}
		if !h.Zaman.IsZero() && (simdi.Sub(h.Zaman) > 24*time.Hour || h.Zaman.Sub(simdi) > 2*time.Hour) {
			continue
		}
		w := haberKelimeler(h.Baslik)
		var hedef *haberKume
		for _, c := range kum {
			if haberBenzer(w, c.kelimeler) {
				hedef = c
				break
			}
		}
		if hedef == nil {
			kum = append(kum, &haberKume{haberOge: h, Kaynaklar: map[string]bool{h.Kaynak: true}, kelimeler: w})
			continue
		}
		hedef.Kaynaklar[h.Kaynak] = true
		for x := range w {
			hedef.kelimeler[x] = true
		}
		if h.Zaman.After(hedef.Zaman) {
			hedef.Zaman = h.Zaman
		}
		// genel kategoriler teknoloji/siberden önce gelir (aynı olay gündemde de varsa orada dursun); v29: teknoloji +
		// siber aynı olay -> siber
		if haberOzel(hedef.Kat) && !haberOzel(h.Kat) {
			hedef.Kat = h.Kat
		} else if hedef.Kat == katTeknoloji && h.Kat == katSiber {
			hedef.Kat = katSiber
		}
	}
	for _, c := range kum {
		c.Puan = float64(len(c.Kaynaklar)) * 10
		k := trKucuk(c.Baslik)
		for _, a := range haberOnemli {
			if strings.Contains(k, a) {
				c.Puan += 3
				break
			}
		}
		if !c.Zaman.IsZero() {
			c.Puan += 4 * (1 - simdi.Sub(c.Zaman).Hours()/24) // daha yeni = biraz daha önde
		}
	}
	sort.SliceStable(kum, func(i, j int) bool { return kum[i].Puan > kum[j].Puan })
	return kum
}

func haberSatir(c *haberKume) string {
	ks := make([]string, 0, len(c.Kaynaklar))
	for k := range c.Kaynaklar {
		ks = append(ks, k)
	}
	sort.Strings(ks)
	return "  • " + noktala(c.Baslik) + " (" + strings.Join(ks, ", ") + ")"
}

// haberBulteni: öne çıkanlar (≥2 kaynak) + kategori başına kalanlar. katBasina=0 -> yalnız öne çıkanlar.
func haberBulteniYaz(kum []*haberKume, oneCikan, katBasina int, yalnizKat string) string {
	var p []string
	kullan := map[*haberKume]bool{}
	if yalnizKat == "" && oneCikan > 0 {
		var o []string
		for _, c := range kum {
			if len(c.Kaynaklar) < 2 || len(o) == oneCikan {
				continue
			}
			o = append(o, haberSatir(c))
			kullan[c] = true
		}
		if len(o) > 0 {
			p = append(p, "🔥 Öne çıkanlar (birden çok kaynakta):")
			p = append(p, o...)
		}
	}
	for _, kat := range haberKategoriSira {
		if katBasina == 0 || (yalnizKat != "" && kat != yalnizKat) {
			continue
		}
		var o []string
		for _, c := range kum {
			if c.Kat != kat || kullan[c] || len(o) == katBasina {
				continue
			}
			o = append(o, haberSatir(c))
			kullan[c] = true
		}
		if len(o) > 0 {
			p = append(p, haberKategoriAd[kat]+":")
			p = append(p, o...)
		}
	}
	return strings.Join(p, "\n")
}

func haberBulteni(ctx context.Context, oneCikan, katBasina int, kat string) (string, error) {
	kum := haberKumele(haberTopla(ctx, kat), time.Now())
	if len(kum) == 0 {
		return "", fmt.Errorf("haber yok")
	}
	return haberBulteniYaz(kum, oneCikan, katBasina, kat), nil
}

// haberKatCoz: /haber argümanı -> kategori ("" = hepsi; ok=false bilinmiyor)
func haberKatCoz(a string) (string, bool) {
	switch trKucuk(strings.TrimSpace(a)) {
	case "":
		return "", true
	case "gundem", "gündem", "siyaset", "politika", "turkiye", "türkiye":
		return katGundem, true
	case "ekonomi", "piyasa", "finans":
		return katEkonomi, true
	case "teknoloji", "tekno", "bilim", "bilim-teknoloji":
		return katTeknoloji, true
	case "dunya", "dünya", "dis", "dış":
		return katDunya, true
	case "siber", "güvenlik", "guvenlik", "siber güvenlik", "siber-guvenlik", "hack": // v29
		return katSiber, true
	}
	return "", false
}

func haberKomutu(ctx context.Context, arg string) string {
	kat, ok := haberKatCoz(arg)
	if !ok {
		return "Bu kategoriyi bilmiyorum efendim. Seçenekler: /haber, /haber siyaset, /haber ekonomi, /haber teknoloji, /haber dunya"
	}
	c, iptal := context.WithTimeout(ctx, 20*time.Second)
	defer iptal()
	var s string
	var err error
	if kat == "" {
		s, err = haberBulteni(c, 4, 3, "")
	} else {
		s, err = haberBulteni(c, 0, 8, kat)
	}
	if err != nil {
		return "Şu an haber kaynaklarına ulaşamadım efendim."
	}
	return "📰 Haberler — " + trTarih(time.Now().In(trZaman)) + "\n" + s
}

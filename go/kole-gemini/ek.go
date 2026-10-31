// ek.go (15.10.2026, Claude): kole-gemini v6 eklentileri (PC'siz):
//   - saat dilimi sabit Türkiye (UTC+3, yaz saati yok): cihazda zoneinfo yok, Go time.Local UTC kalıyordu
//   - hatırlatıcı/alarm: Gemini "YYYY-MM-DD HH:MM | metin" verir, istemci unix zamana çevirir; panel 0.35 saklar ve çalar
//   - namaz vakitleri (aladhan.com, yöntem 13 = Diyanet) ve haber başlıkları (BBC Türkçe RSS) -> --veri ve sesli cevap
//   - sahibin panel notları (/data/notlar.txt) Gemini bağlamına
//   - Telegram: /tmp/bildirim-kuyruk/*.txt -> bot mesajı (jeton /data/asistan/telegram-jeton, sohbet otomatik bulunur)
package main

import (
	"context"
	"encoding/json"
	"encoding/xml"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"time"
)

var trZaman = time.FixedZone("TR", 3*3600)

const hatirlaticiDosya = "/data/asistan/hatirlatici.txt" // panel 0.35 yazar: "unix|tur|metin" (tur h/a/g)

// "2026-10-16 09:00 | ilacını iç" -> unix, metin
func hatirlaticiCoz(deger string) (int64, string, error) {
	p := strings.SplitN(deger, "|", 2)
	z, err := time.ParseInLocation("2006-01-02 15:04", strings.TrimSpace(p[0]), trZaman)
	if err != nil {
		return 0, "", err
	}
	m := ""
	if len(p) == 2 {
		m = strings.Join(strings.Fields(p[1]), " ")
	}
	if r := []rune(m); len(r) > 200 { // v43 (141): UTF-8 harfin ortasından kesilmesin
		m = string(r[:200])
	}
	return z.Unix(), m, nil
}

func hatirlaticiListesi() string {
	b, err := os.ReadFile(hatirlaticiDosya)
	if err != nil || len(strings.TrimSpace(string(b))) == 0 {
		return "Kayıtlı hatırlatıcınız ya da alarmınız yok"
	}
	type hk struct {
		z      int64
		tur, m string
	}
	var l []hk
	for _, s := range strings.Split(string(b), "\n") {
		p := strings.SplitN(s, "|", 3)
		if len(p) != 3 {
			continue
		}
		z, _ := strconv.ParseInt(p[0], 10, 64)
		l = append(l, hk{z, p[1], p[2]})
	}
	sort.Slice(l, func(i, j int) bool { return l[i].z < l[j].z })
	var o []string
	for i, h := range l {
		if i == 4 {
			break
		}
		t := time.Unix(h.z, 0).In(trZaman)
		ad := "hatırlatıcı"
		if h.tur == "a" {
			ad = "alarm"
		} else if h.tur == "g" {
			ad = "her gün alarm"
		}
		s := fmt.Sprintf("%s %s %s", trTarih(t), ad, h.m)
		o = append(o, strings.TrimSpace(s))
	}
	if len(o) == 0 {
		return "Kayıtlı hatırlatıcınız ya da alarmınız yok"
	}
	return fmt.Sprintf("%d kayıt var: %s", len(l), strings.Join(o, "; "))
}

var trAylar = []string{"Ocak", "Şubat", "Mart", "Nisan", "Mayıs", "Haziran", "Temmuz", "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık"}

// "16 Ekim 09:00"
func trTarih(t time.Time) string {
	return fmt.Sprintf("%d %s %s", t.Day(), trAylar[t.Month()-1], t.Format("15:04"))
}

func notlarBaglam() string {
	b, err := os.ReadFile("/data/notlar.txt")
	if err != nil {
		return ""
	}
	s := strings.TrimSpace(string(b))
	if len(s) > 1500 {
		s = s[:1500]
	}
	if s == "" {
		return ""
	}
	return " Sahibinin panel notları (yalnız sorulursa kullan, kendiliğinden okuma):\n" + s
}

// ---------------------------------------------------------------- namaz vakitleri
var vakitAdlari = [][2]string{{"Fajr", "İmsak"}, {"Sunrise", "Güneş"}, {"Dhuhr", "Öğle"}, {"Asr", "İkindi"}, {"Maghrib", "Akşam"}, {"Isha", "Yatsı"}}

func namazGetir(ctx context.Context, k konum) (map[string]string, error) {
	var r struct {
		Data struct {
			Timings map[string]string `json:"timings"`
		} `json:"data"`
	}
	t := time.Now().In(trZaman).Format("02-01-2006")
	u := fmt.Sprintf("https://api.aladhan.com/v1/timings/%s?latitude=%.4f&longitude=%.4f&method=13", t, k.Enlem, k.Boy)
	if err := httpJSON(ctx, u, &r); err != nil {
		return nil, err
	}
	o := map[string]string{}
	for _, v := range vakitAdlari {
		if s, ok := r.Data.Timings[v[0]]; ok && len(s) >= 5 {
			o[v[1]] = s[:5]
		}
	}
	if len(o) < 6 {
		return nil, fmt.Errorf("vakit eksik")
	}
	return o, nil
}

func namazCumlesi(ctx context.Context, deger, evYol string) (string, error) {
	k := evSehri(evYol)
	v, err := namazGetir(ctx, k)
	if err != nil {
		return "", err
	}
	d := strings.ToLower(deger)
	for _, a := range vakitAdlari {
		if d != "" && (strings.Contains(d, strings.ToLower(a[1])) || (a[1] == "Akşam" && strings.Contains(d, "aksam")) ||
			(a[1] == "Öğle" && strings.Contains(d, "ogle")) || (a[1] == "İkindi" && strings.Contains(d, "ikindi")) ||
			(a[1] == "Yatsı" && strings.Contains(d, "yatsi"))) {
			return fmt.Sprintf("%s bugün %s vakti %s", bulunma(k.Ad), strings.ToLower(a[1]), v[a[1]]), nil
		}
	}
	// siradaki vakit
	simdi := time.Now().In(trZaman).Format("15:04")
	for _, a := range vakitAdlari {
		if v[a[1]] > simdi {
			return fmt.Sprintf("%s sıradaki vakit %s, saat %s", bulunma(k.Ad), strings.ToLower(a[1]), v[a[1]]), nil
		}
	}
	return fmt.Sprintf("%s bugünkü vakitler bitti, yarın imsak yaklaşık %s", bulunma(k.Ad), v["İmsak"]), nil
}

// ---------------------------------------------------------------- haberler (RSS)
const haberRSS = "https://feeds.bbci.co.uk/turkce/rss.xml"

func haberGetir(ctx context.Context, n int) ([]string, error) {
	req, _ := http.NewRequestWithContext(ctx, http.MethodGet, haberRSS, nil)
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
	}
	if err := xml.NewDecoder(io.LimitReader(res.Body, 2<<20)).Decode(&r); err != nil {
		return nil, err
	}
	var o []string
	for _, it := range r.Items {
		t := strings.Join(strings.Fields(it.Title), " ")
		if t != "" {
			o = append(o, t)
		}
		if len(o) == n {
			break
		}
	}
	if len(o) == 0 {
		return nil, fmt.Errorf("haber yok")
	}
	return o, nil
}

// haberCumlesi v10: sesli cevap için birden çok kaynakta geçen en önemli 3 başlık (haber.go); olmazsa BBC
func haberCumlesi(ctx context.Context) (string, error) {
	if g, _, _, ok := haberOnbellekOku(time.Now()); ok && len(g) > 0 { // v32: Pano turunun önbelleği (anında)
		return basliklarCumle("Öne çıkan başlıklar: ", g), nil
	}
	var h []string
	genel := func(k haberKaynak) bool { return k.Kat != katSiber } // v29: sesli özet siber akışları çekmez
	for _, c := range haberKumele(haberToplaFn(ctx, genel), time.Now()) {
		if len(h) == 3 {
			break
		}
		h = append(h, c.Baslik)
	}
	if len(h) == 0 {
		var err error
		if h, err = haberGetir(ctx, 3); err != nil {
			return "", err
		}
	}
	for i, t := range h { // "...mi?." gibi cift noktalama olmasin
		if !strings.HasSuffix(t, "?") && !strings.HasSuffix(t, "!") && !strings.HasSuffix(t, ".") {
			h[i] = t + "."
		}
	}
	return "Öne çıkan başlıklar: " + strings.Join(h, " "), nil
}

// ---------------------------------------------------------------- Telegram bildirim kuyrugu
var ( // v42: değişken (testte geçici dizin)
	telegramJeton      = "/data/asistan/telegram-jeton"
	telegramSohbet     = "/data/asistan/telegram-sohbet"
	telegramSahipOgren = "/data/asistan/telegram-sahip-ogren" // v42: yerel izin (kole-telgraf v3 ile aynı kural)
	kuyrukDizin        = "/tmp/bildirim-kuyruk"
)

// tgTaban: Telegram Bot API kökü (v22: testte sahte sunucu)
var tgTaban = "https://api.telegram.org"

func telegramIstek(ctx context.Context, jeton, metod string, form url.Values, v any) error {
	req, _ := http.NewRequestWithContext(ctx, http.MethodPost, tgTaban+"/bot"+jeton+"/"+metod,
		strings.NewReader(form.Encode()))
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<20))
	var g struct {
		Ok          bool            `json:"ok"`
		Description string          `json:"description"`
		Result      json.RawMessage `json:"result"`
	}
	if json.Unmarshal(b, &g) != nil || !g.Ok {
		return fmt.Errorf("telegram %s: HTTP %d %s", metod, res.StatusCode, g.Description)
	}
	if v != nil {
		return json.Unmarshal(g.Result, v)
	}
	return nil
}

// sohbet numarasi: dosyada yoksa YALNIZ yerel izinle (telegramSahipOgren varken) bota özel sohbetten yazılan son
// mesajdan öğrenilir (v42, denetim v2 R2: grup ya da yabancının son mesajı sahip oluyordu); izin, sohbet dosyası
// yazıldıktan sonra silinir
func telegramSohbetBul(ctx context.Context, jeton string) (string, error) {
	if s := oku(telegramSohbet, ""); s != "" {
		return s, nil
	}
	if !dosyaVar(telegramSahipOgren) {
		return "", fmt.Errorf("sohbet yok: %s olusturup bota ozel mesaj yazin", telegramSahipOgren)
	}
	var u []struct {
		Message struct {
			Chat struct {
				ID int64 `json:"id"`
			} `json:"chat"`
		} `json:"message"`
	}
	if err := telegramIstek(ctx, jeton, "getUpdates", url.Values{}, &u); err != nil {
		return "", err
	}
	for i := len(u) - 1; i >= 0; i-- {
		if id := u[i].Message.Chat.ID; id > 0 { // yalnız özel sohbet (grup kimliği negatif)
			s := strconv.FormatInt(id, 10)
			if err := dosyaYazAtomik(telegramSohbet, []byte(s+"\n"), 0600); err != nil {
				return "", fmt.Errorf("sohbet kaydedilemedi: %v", err) // izin durur: sonraki turda yeniden
			}
			_ = os.Remove(telegramSahipOgren)
			return s, nil
		}
	}
	return "", fmt.Errorf("sohbet yok: bota Telegram'dan ozel mesaj yazin")
}

// kuyruktaki her .txt (ilk satir metin; "foto:<yol>" satiri varsa fotograf) gonderilir, basariliysa silinir
func bildirimGonder() error {
	jeton := oku(telegramJeton, "")
	if jeton == "" {
		return fmt.Errorf("telegram jetonu yok")
	}
	ctx, iptal := context.WithTimeout(context.Background(), 40*time.Second)
	defer iptal()
	sohbet, err := telegramSohbetBul(ctx, jeton)
	if err != nil {
		return err
	}
	dosyalar, _ := filepath.Glob(filepath.Join(kuyrukDizin, "*.txt"))
	sort.Strings(dosyalar)
	for _, f := range dosyalar {
		b, err := os.ReadFile(f)
		if err != nil {
			continue
		}
		metin, foto := "", ""
		for _, s := range strings.Split(string(b), "\n") {
			if v, ok := strings.CutPrefix(s, "foto:"); ok {
				foto = strings.TrimSpace(v)
			} else if strings.TrimSpace(s) != "" {
				metin += s + "\n"
			}
		}
		metin = strings.TrimSpace(metin)
		if st, e := os.Stat(foto); foto != "" && (e != nil || st.Size() == 0 || st.Size() > 5<<20) {
			foto = "" // v42: foto yok/boş/5 MB üstü: yalnız metin (HTTP kodsuz hata kuyruğu sonsuza dek tıkamasın)
		}
		if foto != "" {
			err = telegramFoto(ctx, jeton, sohbet, foto, metin)
		} else if metin != "" {
			err = telegramIstek(ctx, jeton, "sendMessage", url.Values{"chat_id": {sohbet}, "text": {metin}}, nil)
		}
		if err != nil && !tgDosyaHatasi(err) {
			return err // ağ, 5xx, 429, jeton/sohbet: dosya durur, sonraki turda yeniden
		}
		if err != nil { // v42 (denetim v2 #20): Telegram bu dosyayı reddetti; tanı için .bozuk, sıradakiler gider
			fmt.Fprintln(os.Stderr, "bildirim atlandı:", filepath.Base(f), err)
			_ = os.Rename(f, f+".bozuk")
		} else {
			_ = os.Remove(f)
		}
		if foto != "" && strings.HasPrefix(foto, "/tmp/") {
			_ = os.Remove(foto)
		}
	}
	return nil
}

var tgHTTPKodRe = regexp.MustCompile(`HTTP (\d{3})`)

// tgDosyaHatasi v42: Telegram bu isteği reddetti (4xx); aynı dosya yeniden gönderilse de gitmez. 401/403/404 (jeton ya
// da engellenmiş sohbet: bütün dosyalar aynı) ve 429 (yavaşla) dosyanın suçu değil: false.
func tgDosyaHatasi(err error) bool {
	m := tgHTTPKodRe.FindStringSubmatch(err.Error())
	if m == nil {
		return false
	}
	k, _ := strconv.Atoi(m[1])
	return k >= 400 && k < 500 && k != 401 && k != 403 && k != 404 && k != 429
}

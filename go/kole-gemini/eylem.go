// eylem.go v12 (23.10.2026, Claude; kole-gemini v35): hey_kole 'kapat' -> {"heykole": false} (panel 0.75).
// eylem.go v11 (22.10.2026, Claude; kole-gemini v34): zaman_atlamali + aralık (1..3600 sn) ve olay_ac/olay_kapat (panel 0.74).
// eylem.go v10 (22.10.2026, Claude; kole-gemini v33): eylemEnum (haber önbelleği tazeyken "haber" yok).
// eylem.go v9 (22.10.2026, Claude; kole-gemini v32): "ses" (panelin hoparlör sesi, panel 0.73); haber deger "siber".
// eylem.go v8 (15.10.2026, Claude): ekran_koruyucu "cerceve" (panel 0.37 fotoğraf çerçevesi).
// eylem.go v7 (15.10.2026, Claude): + foto, bekci, zaman_atlamali (panel 0.36 kamera bayraklari).
// eylem.go v6 (15.10.2026, Claude): + hatirlatici/alarm/hatirlatici_sil (panel 0.35), bilgi: hatirlatici_listesi, namaz, haber.
// eylem.go v5 (15.10.2026, Claude): isik = ekran parlakligi, goreli parlaklik (+N/-N -> br_rel).
// eylem.go v4 (15.10.2026, Claude): + muzik (PC medya), hava/doviz (bilgi, istemci cevaplar).
// eylem.go (15.10.2026, Claude): Gemini'nin önerdiği panel işlemlerini DOĞRULAR ve panel 0.32'nin okuduğu
// /tmp/asistan-eylem.json'a ({"seq":<ns>,"ui":[{...},...]}) yazar. Yalnız izinli işlemler; değerler sınırlı.
// kullanıcının seçimi (15.10): ekran ve görünüm, sayfalar ve notlar, zamanlayıcı, kamera kaydı ve gizlilik kalkanı (yalnız AÇMA).
package main

import (
	"encoding/json"
	"strconv"
	"strings"
	"time"
)

type eylem struct {
	Ad    string `json:"ad"`
	Deger string `json:"deger"`
}

// Gemini şemasındaki enum (bunların dışındaki adlar zaten üretilemez; yine de aşağıda tek tek doğrulanır)
var eylemAdlari = []string{"parlaklik", "ses", "tema", "ekran_koruyucu", "kilit", "rahatsiz_etme", "sayfa", "not_ekle",
	"zamanlayici", "kayit", "kalkan", "muzik", "hava", "doviz", "hatirlatici", "alarm", "hatirlatici_sil",
	"hatirlatici_listesi", "namaz", "haber", "foto", "bekci", "zaman_atlamali", "hey_kole"}

const eylemTalimat = "Sahibin panelde bir işlem isterse 'eylemler' listesine ekle ve cevabında yaptığını kısaca söyle " +
	"(örnek: 'Parlaklığı yüzde otuz yaptım efendim.'). İşlem istenmiyorsa 'eylemler' boş olsun. İzinli işlemler ve 'deger':\n" +
	"- parlaklik: 5..100 arası yüzde, 'oto' (otomatik parlaklık) ya da göreli '+20' / '-20' ('biraz artır' +20, 'biraz azalt' -20, " +
	"'çok' ±40). 'Işık', 'ekran ışığı', 'ışığı kıs/aç', 'karart', 'aydınlat' hep EKRAN PARLAKLIĞI demektir (odada akıllı ışık yok).\n" +
	"- ses: PANELİN KENDİ hoparlör sesi: 0..100 yüzde, göreli '+20' / '-20' ('sesi biraz kıs' -20, 'biraz aç' +20, 'çok' ±40), " +
	"'sessiz' (sesi kapat / sustur) ya da 'ac' (sessizden çık). 'Ses', 'hoparlör', 'sesi kıs/aç/kapat' hep bunu demektir.\n" +
	"- tema: 'mavi', 'kirmizi' (kırmızı-siyah) ya da 'beyaz'\n" +
	"- ekran_koruyucu: 'ac', 'robot' (robot göz), 'saat' ya da 'cerceve' (SD karttaki fotoğraflar; 'fotoğrafları göster', 'çerçeve')\n" +
	"- kilit: 'ac' (paneli kilitler; açmak yalnız dokunarak)\n" +
	"- rahatsiz_etme: 'ac' ya da 'kapat'\n" +
	"- sayfa: 'pano', 'medya', 'kisayollar', 'asistan', 'zamanlayici', 'notlar', 'monitor', 'pc' (PC ekranı), 'gizlilik', " +
	"'ayarlar', ya da ayar bölümü 'ayar:ekran', 'ayar:ses', 'ayar:kole', 'ayar:baglanti', 'ayar:gizlilik', 'ayar:sistem'\n" +
	"- not_ekle: not metni (kısa, sahibin söylediği gibi)\n" +
	"- zamanlayici: dakika (1..180) ile kurup başlatır; ya da 'dur', 'devam', 'sifirla'\n" +
	"- kayit: 'baslat' ya da 'durdur' (kameranın SD kartına kaydı)\n" +
	"- kalkan: 'ac' (gizlilik kalkanı; kapatmak YALNIZ panelden dokunarak, sesle kapatılamaz — istenirse bunu söyle)\n" +
	"- muzik: 'oynat_duraklat', 'sonraki', 'onceki', 'ses_ac', 'ses_kis', 'sessiz' (bilgisayarda çalan müzik; PC kapalıysa çalışmaz; " +
	"ses_ac/ses_kis/sessiz YALNIZ 'bilgisayarın sesi' ya da 'müziğin sesi' denirse, yoksa 'ses' kullan)\n" +
	"- hava: şehir adı (sahibin şehri için boş bırak); yarın soruluyorsa sonuna ' yarın' ekle. Cevap alanına kısa bir şey yaz, " +
	"gerçek hava bilgisi senin yerine eklenecek, SAKIN hava uydurma.\n" +
	"- doviz: 'dolar', 'euro', 'sterlin' ya da boş (dolar ve euro). Kur UYDURMA, eklenecek.\n" +
	"- hatirlatici: 'YYYY-AA-GG SS:DD | ne hatırlatılacak' (şu anki tarih/saate göre MUTLAK zaman hesapla; 'yarın 9'da' gibi)\n" +
	"- alarm: 'YYYY-AA-GG SS:DD' ya da her gün için 'her gün SS:DD'\n" +
	"- hatirlatici_sil: 'hepsi' ya da 'son'\n" +
	"- hatirlatici_listesi: boş (kayıtlı hatırlatıcı/alarmları söyler, gerçek liste eklenecek)\n" +
	"- namaz: vakit adı ('akşam', 'öğle'...) ya da boş (sıradaki vakit). Vakit UYDURMA, eklenecek.\n" +
	"- haber: boş (genel başlıklar) ya da 'siber' (siber güvenlik başlıkları); başlıklar eklenecek, UYDURMA. Talimatın sonunda " +
	"'Güncel haber başlıkları' verildiyse haber eylemi EKLEME: o başlıklardan 2-3 cümlelik Türkçe özet yap (İngilizce " +
	"başlıkları Türkçe anlat)\n" +
	"- foto: 'telegram' (kameradan fotoğraf çekip sahibin Telegram'ına gönderir; 'fotoğraf çek' denince bunu kullan)\n" +
	"- bekci: 'ac' ya da 'kapat' (kamera hareket görünce Telegram'a fotoğraflı bildirim)\n" +
	"- zaman_atlamali: 'baslat' ya da 'durdur' (time-lapse: kamera aralıklarla kare biriktirir); kare aralığı saniye olarak " +
	"'1'..'3600' ('her 3 saniyede bir' -> '3', 'dakikada bir' -> '60'); 'olay_ac' ya da 'olay_kapat' (time-lapse olay " +
	"tespiti: hareket anları ek kare + Telegram fotoğrafı). Birden çok istenirse her biri için ayrı eylem ekle.\n" +
	"- hey_kole: 'kapat' (Hey Köle dinlemesini kapatır, mikrofonu bırakır: 'dinlemeyi kapat', 'beni dinleme', 'Hey Köle'yi " +
	"kapat' denince. 'Sesi kapat' bu DEĞİL, o 'ses' eylemi). Sesle yeniden AÇILAMAZ: açmak için panelde Denetim Merkezi'ndeki " +
	"Hey Köle düğmesi ya da Telegram'da /heykole ac; kapatırken bunu da söyle.\n" +
	"Telefon araması, mesaj gönderme, takvim, alışveriş gibi listede olmayan işlemleri yapamazsın; istenirse nazikçe söyle. " +
	"Hatırlatıcılar ve alarmlar zamanı gelince panelde ve sahibin Telegram'ında bildirilir."

var sayfalar = map[string]bool{"pano": true, "medya": true, "kisayollar": true, "asistan": true, "zamanlayici": true,
	"notlar": true, "monitor": true, "pc": true, "gizlilik": true, "ayarlar": true}
var ayarBolumleri = map[string]bool{"ekran": true, "ses": true, "kole": true, "baglanti": true, "gizlilik": true, "sistem": true}

// eylemEnum v33: şema enum'u; haber bağlamı varsa "haber" çıkarılır (model başlıkları kendisi Türkçe özetler)
func eylemEnum(haberBaglamVar bool) []string {
	if !haberBaglamVar {
		return eylemAdlari
	}
	o := make([]string, 0, len(eylemAdlari))
	for _, a := range eylemAdlari {
		if a != "haber" {
			o = append(o, a)
		}
	}
	return o
}

// tek işlem -> panel ui nesnesi (nil: geçersiz)
func eylemUi(e eylem) map[string]any {
	d := strings.ToLower(strings.TrimSpace(e.Deger))
	switch e.Ad {
	case "parlaklik":
		if d == "oto" || d == "otomatik" {
			return map[string]any{"br_mode": "auto"}
		}
		if strings.HasPrefix(d, "+") || strings.HasPrefix(d, "-") { // goreli (panel 0.34 br_rel)
			n, err := strconv.Atoi(strings.TrimSuffix(d, "%"))
			if err != nil || n == 0 {
				return nil
			}
			if n > 95 {
				n = 95
			}
			if n < -95 {
				n = -95
			}
			return map[string]any{"br_rel": n}
		}
		n, err := strconv.Atoi(strings.TrimSuffix(strings.TrimPrefix(d, "%"), "%"))
		if err != nil {
			return nil
		}
		if n < 5 {
			n = 5
		}
		if n > 100 {
			n = 100
		}
		return map[string]any{"br_mode": "manual", "br_pct": n}
	case "tema":
		t := map[string]int{"mavi": 0, "kirmizi": 1, "kırmızı": 1, "beyaz": 2}
		if v, ok := t[d]; ok {
			return map[string]any{"theme": v}
		}
	case "ekran_koruyucu":
		switch d {
		case "ac":
			return map[string]any{"ss": 1}
		case "robot", "saat", "cerceve":
			return map[string]any{"ss": d}
		case "çerçeve":
			return map[string]any{"ss": "cerceve"}
		}
	case "kilit":
		if d == "ac" {
			return map[string]any{"kilit": true}
		}
	case "rahatsiz_etme":
		if d == "ac" || d == "kapat" {
			return map[string]any{"dnd": d == "ac"}
		}
	case "sayfa":
		if b, ok := strings.CutPrefix(d, "ayar:"); ok && ayarBolumleri[b] {
			return map[string]any{"ayar": b}
		}
		if sayfalar[d] {
			return map[string]any{"sayfa": d}
		}
	case "not_ekle":
		m := strings.Join(strings.Fields(e.Deger), " ")
		if m != "" && len(m) <= 300 {
			return map[string]any{"not_ekle": m}
		}
	case "zamanlayici":
		if d == "dur" || d == "devam" || d == "sifirla" {
			return map[string]any{"tm": d}
		}
		if n, err := strconv.Atoi(d); err == nil && n >= 1 && n <= 180 {
			return map[string]any{"tm": n}
		}
	case "kayit":
		if d == "baslat" || d == "durdur" {
			return map[string]any{"kayit": d}
		}
	case "ses": // v32: panelin hoparlör sesi (panel 0.73: "ses", "ses_rel", "sessiz")
		switch d {
		case "sessiz", "kapat", "sustur":
			return map[string]any{"sessiz": true}
		case "ac", "aç":
			return map[string]any{"sessiz": false}
		}
		if strings.HasPrefix(d, "+") || strings.HasPrefix(d, "-") {
			n, err := strconv.Atoi(strings.TrimSuffix(d, "%"))
			if err != nil || n == 0 {
				return nil
			}
			if n > 100 {
				n = 100
			}
			if n < -100 {
				n = -100
			}
			return map[string]any{"ses_rel": n}
		}
		n, err := strconv.Atoi(strings.TrimSuffix(strings.TrimPrefix(d, "%"), "%"))
		if err != nil {
			return nil
		}
		if n < 0 {
			n = 0
		}
		if n > 100 {
			n = 100
		}
		return map[string]any{"ses": n}
	case "muzik":
		m := map[string]string{"oynat_duraklat": "playpause", "oynat": "playpause", "duraklat": "playpause",
			"sonraki": "next", "onceki": "prev", "ses_ac": "volup", "ses_kis": "voldown", "sessiz": "mute"}
		if id, ok := m[d]; ok {
			return map[string]any{"press": "macro:" + id}
		}
	case "hatirlatici":
		if z, m, err := hatirlaticiCoz(e.Deger); err == nil && z > time.Now().Unix() && m != "" {
			return map[string]any{"hatirlat": map[string]any{"z": z, "t": "h", "m": m}}
		}
	case "alarm":
		if s, ok := strings.CutPrefix(d, "her gün"); ok || strings.HasPrefix(d, "her gun") {
			if !ok {
				s = strings.TrimPrefix(d, "her gun")
			}
			if hm, err := time.ParseInLocation("15:04", strings.TrimSpace(s), trZaman); err == nil {
				n := time.Now().In(trZaman)
				z := time.Date(n.Year(), n.Month(), n.Day(), hm.Hour(), hm.Minute(), 0, 0, trZaman)
				if !z.After(n) {
					z = z.Add(24 * time.Hour)
				}
				return map[string]any{"hatirlat": map[string]any{"z": z.Unix(), "t": "g", "m": "Her gün alarm"}}
			}
			return nil
		}
		if z, _, err := hatirlaticiCoz(e.Deger); err == nil && z > time.Now().Unix() {
			return map[string]any{"hatirlat": map[string]any{"z": z, "t": "a", "m": "Alarm"}}
		}
	case "hatirlatici_sil":
		if d == "hepsi" || d == "son" {
			return map[string]any{"hatirlat_sil": d}
		}
	case "foto":
		return map[string]any{"kamera_foto": "telegram"}
	case "bekci":
		if d == "ac" || d == "kapat" {
			return map[string]any{"bekci": d}
		}
	case "zaman_atlamali":
		if d == "baslat" || d == "durdur" {
			return map[string]any{"zaman": d}
		}
		if d == "olay_ac" || d == "olay_kapat" { // v34 (panel 0.74)
			return map[string]any{"zaman_olay": strings.TrimPrefix(d, "olay_")}
		}
		if n := zamanAralikCoz(d); n > 0 {
			return map[string]any{"zaman_aralik": n}
		}
	case "hey_kole": // v35: yalnız kapatma
		if d == "kapat" {
			return map[string]any{"heykole": false}
		}
	case "kalkan":
		if d == "ac" { // kapatma sesle YOK
			return map[string]any{"kalkan": "ac"}
		}
	}
	return nil
}

// geçerli işlemleri dosyaya yazar; uygulanan sayısını döndürür
func eylemleriYaz(es []eylem, yol string) int {
	var ui []map[string]any
	for _, e := range es {
		if len(ui) >= 4 {
			break
		}
		if u := eylemUi(e); u != nil {
			ui = append(ui, u)
		}
	}
	if len(ui) == 0 {
		return 0
	}
	b, _ := json.Marshal(map[string]any{"seq": time.Now().UnixNano(), "ui": ui})
	if dosyaYazAtomik(yol, b, 0644) != nil { // v42: benzersiz geçici ad (iki süreç aynı anda)
		return 0
	}
	return len(ui)
}

// telegram.go v11 (21.10.2026, Claude; kole-gemini v22): komutlar tgKomut (yazı / kalıcı düğme / satır içi düğme),
// callback_query, menü kurulumu ve logo (telegram_menu.go), uzun sorgu (tur başına en çok 2 x 20 sn).
// telegram.go v10 (19.10.2026, Claude): + /haber [kategori] (haber.go), ayrıntılı /yardim, ezan/namaz çıkarıldı.
// telegram.go v9 (15.10.2026, Claude): + /saglik, /sabah, /durum saglikli; her turda zamanlanmisIsler (saglik.go).
// telegram.go v8 (15.10.2026, Claude): + sahibin gönderdiği fotoğraf (sıkıştırılmış ya da JPEG/PNG dosya) -> dijital
// çerçeve /mnt/sd/fotograf/tg-*.jpg (cerceve.go; progresif/PNG baseline'a çevrilir), /cerceve [ac|kapat|goster].
// telegram.go (15.10.2026, Claude): kole-gemini v7 iki yönlü Telegram (--telegram: kuyruğu gönder + gelen mesajları işle).
// YALNIZ sahibin sohbeti (/data/asistan/telegram-sohbet) ve v45 izinliler (/data/asistan/telegram-izinli) kabul edilir.
//
//	/foto            -> /tmp/kamera-foto-iste "telegram" (kamera-kayit v3 fotoğrafı kuyruğa koyar, sonraki turda gelir)
//	/bekci ac|kapat  -> /mnt/sd/kamera/bekci-acik
//	/zaman ac|kapat  -> /mnt/sd/kamera/zaman-acik (zaman atlamalı)
//	/cerceve [ac|kapat|goster] -> panel çerçeve ayarı / hemen göster ({"cerceve":..}, {"ss":"cerceve"})
//	fotoğraf         -> çerçeve klasörüne kaydedilir (açıklamada "goster" varsa hemen gösterilir)
//	/durum           -> hava + kamera + hatırlatıcı özeti
//	/yardim, /start  -> komut listesi
//	düz metin        -> Köle'ye soru (kendini --metin ile çağırır; işlemler panele aynı yoldan gider)
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"
)

const (
	telegramOffset = "/data/asistan/telegram-offset"
	bekciBayrak    = "/mnt/sd/kamera/bekci-acik"
	zamanBayrak    = "/mnt/sd/kamera/zaman-acik"
	kalkanBayrak   = "/tmp/gizlilik-kalkani"
)

// yardimMetni v10: ayrıntılı, bölümlü (kullanıcı: "botta yapılabilecek şeyleri daha detaylı yazsın"); ezan/namaz çıkarıldı
const yardimMetni = "🤖 VeLinux KÖLE — emrinizdeyim efendim. Yapabildiklerim:\n\n" +
	"👆 TEK DOKUNUŞ: alttaki düğmeler ya da soldaki Menü. Düğmeler görünmüyorsa /menu yazın.\n\n" +
	"📰 HABERLER\n" +
	"/haber — öne çıkanlar (birden çok kaynakta geçenler) + gündem, ekonomi, teknoloji, siber güvenlik, dünya\n" +
	"/haber siyaset | ekonomi | teknoloji | siber | dunya — tek kategori, 8 başlık\n" +
	"Kaynaklar: AA, TRT, NTV, BBC Türkçe; teknolojide ShiftDelete, Webtekno, DonanımHaber; siber güvenlikte ShiftDelete, " +
	"BleepingComputer, The Hacker News, SecurityWeek, Dark Reading. Magazin/spor elenir.\n\n" +
	"☀️ SABAH ÖZETİ (her gün, varsayılan 08:00)\n" +
	"Hava + giyim önerisi, şehrinizdeki önemli durumlar (fırtına, sel, yerel haber), haberler, döviz, hatırlatıcılar, cihaz durumu.\n" +
	"/sabah — şimdi gönder | /sabah 07:30 — saati değiştir | /sabah kapat | /sabah ac\n\n" +
	"📍 KONUM\n" +
	"/sehir — şu anki şehir | /sehir İzmir — değiştir | /sehir ev — varsayılana dön\n\n" +
	"📷 KAMERA VE GÜVENLİK\n" +
	"/foto — anlık fotoğraf\n" +
	"/canli — Telegram'da canlı kamera (60 sn; ⏹ durdur, ➕ 60 sn uzat, en çok 5 dk; akıcı izleme: KÖLE Merkez)\n" +
	"/bekci — durum ve Aç/Kapat düğmeleri | /bekci ac | kapat — hareket görünce fotoğraflı bildirim\n" +
	"/zaman — durum ve düğmeler | /zaman ac | kapat — zaman atlamalı çekim (time-lapse)\n" +
	"/zaman 3 — kare aralığı sn (1..3600; 1dk da olur) | /zaman olay ac | kapat — hareket anları ek kare + fotoğraf\n" +
	"Gizlilik kalkanı paneldeyken kamera hiçbir yoldan açılmaz.\n\n" +
	"🖼 DİJİTAL ÇERÇEVE\n" +
	"Bana fotoğraf gönderin, çerçeveye eklerim (açıklamaya 'göster' yazarsanız hemen gösteririm).\n" +
	"/cerceve — düğmeler | /cerceve goster — şimdi | /cerceve ac | /cerceve kapat — ekran koruyucu olarak\n\n" +
	"🔊 SES (Hey Köle sesli cevap)\n" +
	"/ses merhaba — hoparlörden söylerim\n" +
	"/sesler — sesleri listele | /sesler Leda — sesi değiştir (örnek okurum)\n" +
	"/heykole — Hey Köle dinlemesi: durum ve Aç/Kapat | /heykole ac | kapat (kapalıyken mikrofon bırakılır)\n" +
	"/seskapat | /sesac — Hey Köle cevaplarını sesli söyleme\n" +
	"/sesdurum — ses, hazır cümleler, sağlayıcılar, son kota hatası\n" +
	"/dusunme — Hey Köle düşünme seviyesi (hızlı / dengeli / dikkatli) | /hiz — son cevap süreleri\n" +
	"/anahtar — yedek ses servisleri (Azure, Google Cloud, ElevenLabs) ve AssemblyAI (konuşma → metin); mesajınız silinir\n\n" +
	"🩺 CİHAZ\n" +
	"/durum — hava, kamera, hatırlatıcılar ve hizmetler\n" +
	"/saglik — hizmetler, bellek, Wi-Fi hataları, SD kart, kamera saati\n" +
	"Bir hizmet durursa ya da geri gelirse kendiliğimden haber veririm.\n\n" +
	"💬 NORMAL YAZIN (yapay zekâ anlar)\n" +
	"• Ekran: 'parlaklığı %40 yap', 'ışığı biraz kıs', 'temayı beyaz yap', 'robot gözü aç', 'paneli kilitle'\n" +
	"• Sayfa: 'PC sayfasını aç', 'ayarlar ekran bölümüne git'\n" +
	"• Hatırlatma: 'yarın 9'da doktoru hatırlat', 'her gün 07:00 alarm kur', 'hatırlatıcılarımı söyle', 'sonuncuyu sil'\n" +
	"• Zamanlayıcı ve not: '25 dakikalık zamanlayıcı başlat', 'not al: süt alınacak'\n" +
	"• Bilgi: 'yarın Ankara'da hava nasıl', 'dolar kaç', 'günün haberleri'\n" +
	"• Bilgisayar müziği: 'sonraki şarkı', 'müziği duraklat', 'sesi kıs'\n" +
	"• Rahatsız etme: 'rahatsız etme modunu aç'\n" +
	"Telefon araması, mesaj gönderme ve alışveriş yapamam."

func dosyaVar(y string) bool { _, err := os.Stat(y); return err == nil }

func bayrak(y string, ac bool) error {
	if ac {
		return os.WriteFile(y, nil, 0644)
	}
	if err := os.Remove(y); err != nil && !os.IsNotExist(err) {
		return err
	}
	return nil
}

func durumOzeti(sehirYol string) string {
	var p []string
	ctx, iptal := context.WithTimeout(context.Background(), 10*time.Second)
	defer iptal()
	if s, err := havaCumlesi(ctx, "", sehirYol); err == nil {
		p = append(p, s+".")
	}
	var kd struct {
		Kayit    bool  `json:"kayit"`
		Bekci    bool  `json:"bekci"`
		Zaman    bool  `json:"zaman"`
		Kalkan   bool  `json:"kalkan"`
		SonAlarm int64 `json:"son_alarm"`
	}
	if b, err := os.ReadFile("/tmp/kamera-kayit.json"); err == nil {
		_ = json.Unmarshal(b, &kd)
	}
	ac := func(v bool) string {
		if v {
			return "açık"
		}
		return "kapalı"
	}
	k := fmt.Sprintf("Kamera: kayıt %s, bekçi %s, zaman atlamalı %s", ac(kd.Kayit), ac(kd.Bekci || dosyaVar(bekciBayrak)), zamanKisa(zamanY, kd.Zaman)) // v34
	if dosyaVar(kalkanBayrak) {
		k += " (gizlilik kalkanı AÇIK)"
	}
	if kd.SonAlarm > 0 {
		k += ", son hareket " + trTarih(time.Unix(kd.SonAlarm, 0).In(trZaman))
	}
	p = append(p, k+".", heyKoleKisa()+".", hatirlaticiListesi()+".") // v35: Hey Köle satırı
	return strings.Join(p, "\n")
}

// v51: düz cevaplar kalıcı menüyle gider: 🗾 Gezi'nin force_reply istemi menüyü gizliyordu, bir sonraki cevapta geri gelir
func telegramYanit(ctx context.Context, jeton, sohbet, metin string) {
	kb, _ := json.Marshal(tgKlavyeJSON())
	if err := telegramIstek(ctx, jeton, "sendMessage", url.Values{"chat_id": {sohbet}, "text": {metin},
		"reply_markup": {string(kb)}}, nil); err != nil {
		fmt.Fprintln(os.Stderr, "telegram yanit:", err)
	}
}

// serbest metni Köle'ye sor: kendini --metin ile çağır (aynı Gemini akışı, işlemler panele)
func koleyeSor(metin string) string {
	if sunucuYollar.anahtar != "" { // v40: aynı süreçte (Go süreci açılmaz)
		s, rc := soruIsleKilitli(sunucuYollar, soruIstek{Metin: metin}, time.Now())
		if rc != 0 || s.Cevap == "" {
			return "Üzgünüm efendim, şu an bağlantı kuramıyorum."
		}
		return s.Cevap
	}
	ben, err := os.Executable()
	if err != nil {
		return "Şu an cevap veremiyorum efendim."
	}
	ctx, iptal := context.WithTimeout(context.Background(), 40*time.Second)
	defer iptal()
	out, err := exec.CommandContext(ctx, ben, "--metin", metin).Output()
	if err != nil {
		return "Üzgünüm efendim, şu an bağlantı kuramıyorum."
	}
	var s sonuc
	if json.Unmarshal(out, &s) != nil || s.Cevap == "" {
		return "Üzgünüm efendim, cevabı anlayamadım."
	}
	return s.Cevap
}

// telegramIndir: getFile + dosya indirme (en çok sinir bayt); jeton hata metnine yazılmaz
func telegramIndir(ctx context.Context, jeton, dosyaID string, sinir int64) ([]byte, error) {
	var f struct {
		FilePath string `json:"file_path"`
		FileSize int64  `json:"file_size"`
	}
	if err := telegramIstek(ctx, jeton, "getFile", url.Values{"file_id": {dosyaID}}, &f); err != nil {
		return nil, err
	}
	if f.FilePath == "" || f.FileSize > sinir {
		return nil, fmt.Errorf("dosya alınamadı ya da çok büyük")
	}
	req, _ := http.NewRequestWithContext(ctx, http.MethodGet, tgTaban+"/file/bot"+jeton+"/"+f.FilePath, nil)
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	defer res.Body.Close()
	if res.StatusCode != 200 {
		return nil, fmt.Errorf("indirme HTTP %d", res.StatusCode)
	}
	b, err := io.ReadAll(io.LimitReader(res.Body, sinir+1))
	if err != nil {
		return nil, fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	if int64(len(b)) > sinir {
		return nil, fmt.Errorf("dosya çok büyük")
	}
	return b, nil
}

// panele tek işlem gönder (/tmp/asistan-eylem.json; panel 0.37 ey_izinli: "ss", "cerceve")
func panelIslem(ui map[string]any) bool {
	b, _ := json.Marshal(map[string]any{"seq": time.Now().UnixNano(), "ui": []map[string]any{ui}})
	return dosyaYazAtomik("/tmp/asistan-eylem.json", b, 0644) == nil // v42: eylemleriYaz ile aynı ".part" adı paylaşılıyordu
}

type tgFoto struct {
	FileID   string `json:"file_id"`
	FileSize int64  `json:"file_size"`
	Width    int    `json:"width"`
	Height   int    `json:"height"`
}

type tgBelge struct {
	FileID   string `json:"file_id"`
	FileName string `json:"file_name"`
	MimeType string `json:"mime_type"`
	FileSize int64  `json:"file_size"`
}

// fotoğraf mesajını çerçeveye koy; sahibe verilecek cevabı döndürür ("" = fotoğraf yok)
func fotoMesaji(ctx context.Context, jeton string, foto []tgFoto, belge *tgBelge, aciklama string) string {
	id := ""
	if len(foto) > 0 { // en büyük boyut
		en := foto[0]
		for _, f := range foto[1:] {
			if f.Width*f.Height > en.Width*en.Height {
				en = f
			}
		}
		id = en.FileID
	} else if belge != nil && belge.FileID != "" {
		m := strings.ToLower(belge.MimeType)
		if m != "image/jpeg" && m != "image/png" {
			return "Yalnız JPEG ya da PNG fotoğrafları çerçeveye ekleyebilirim efendim."
		}
		if belge.FileSize > cerceveAzami {
			return "Bu dosya çok büyük efendim (en çok 10 MB)."
		}
		id = belge.FileID
	} else {
		return ""
	}
	veri, err := telegramIndir(ctx, jeton, id, cerceveAzami)
	if err != nil {
		fmt.Fprintln(os.Stderr, "telegram indir:", err)
		return "Fotoğrafı indiremedim efendim."
	}
	yol, islem, err := cerceveKaydet(cerceveDizin, veri, time.Now().In(trZaman))
	if err != nil {
		fmt.Fprintln(os.Stderr, "cerceve:", err)
		return "Fotoğrafı kaydedemedim efendim: " + err.Error() + "."
	}
	fmt.Fprintf(os.Stderr, "cerceve: %s (%s, %d bayt)\n", yol, islem, len(veri))
	cevap := fmt.Sprintf("🖼 Fotoğrafı çerçeveye ekledim efendim (klasörde %d fotoğraf).", cerceveSay(cerceveDizin))
	a := strings.ToLower(aciklama)
	if strings.Contains(a, "goster") || strings.Contains(a, "göster") {
		if panelIslem(map[string]any{"ss": "cerceve"}) {
			cevap += " Panelde gösteriyorum."
		}
	}
	return cevap
}

// tgGuncelleme v11: mesaj ya da satır içi düğme (callback_query)
type tgGuncelleme struct {
	UpdateID int64 `json:"update_id"`
	Message  struct {
		MessageID  int64     `json:"message_id"` // v17: /anahtar mesaji silinir
		Text       string    `json:"text"`
		Caption    string    `json:"caption"`
		Photo      []tgFoto  `json:"photo"`
		Document   *tgBelge  `json:"document"`
		Yanitlanan *struct { // v48: bota verilen yanıt (🗾 Gezi isteminden soru)
			Text string `json:"text"`
		} `json:"reply_to_message"`
		Chat struct {
			ID int64 `json:"id"`
		} `json:"chat"`
	} `json:"message"`
	CallbackQuery *struct {
		ID      string `json:"id"`
		Data    string `json:"data"`
		Message *struct {
			Chat struct {
				ID int64 `json:"id"`
			} `json:"chat"`
		} `json:"message"`
		From struct {
			ID int64 `json:"id"`
		} `json:"from"`
	} `json:"callback_query"`
}

// v25 (scripts/447; Sıra 8 madde 55): autorun v32 her çıkıştan sonra 15 sn uyuyup YENİ süreç başlatıyordu (v24 süreci
// ~41 sn yaşar: saatte ~64 başlatma, her birinde çalışma zamanı + kök sertifika + TLS el sıkışma; uykuda düğmeler 15 sn'ye
// kadar bekliyordu). v25 v24'ün turunu (telegramTur, AYNEN) aynı süreçte tgSurecButce boyunca tekrarlar; bağlantı açık kalır.
// Hata -> eskisi gibi çık (autorun 15 sn sonra yeniden dener); art arda 3 kısa tur (409 çakışması / ağ) -> çık.
var (
	tgSurecButce = 10 * time.Minute // bir --telegram sürecinin en uzun ömrü
	tgTurPayi    = time.Minute      // bir turun en uzun süresi (telegramTur bağlamı 60 sn)
	tgEnKisaTur  = 5 * time.Second
	tgTurFn      = telegramTur
)

func telegramDinle(sehirYol string) error {
	defer soruSunucusu(sunucuYollar)() // v40: Hey Köle / --metin soruları bu süreçte; dönüşte süren işleri bekler
	defer veriBaslat(sehirYol)()       // v28: Pano verisi bu süreçte (veri.go)
	canliEskiTemizle()                 // v30: önceki süreç izleme ortasında öldüyse kalan bayrak
	defer canliBitmesiniBekle()        // v30: dönüşte süren canlı izlemeyi (en çok 5 dk) yarıda kesme
	bas, kisa := time.Now(), 0
	for {
		t0 := time.Now()
		if err := tgTurFn(sehirYol); err != nil {
			return err
		}
		if time.Since(t0) < tgEnKisaTur {
			if kisa++; kisa >= 3 {
				return nil
			}
		} else {
			kisa = 0
		}
		if time.Since(bas)+tgTurPayi > tgSurecButce && !canliSurdu() { // v30: canlı izleme sürerken süreç kapanmaz
			return nil
		}
	}
}

// telegramTur: v24 telegramDinle gövdesi (tek tur: kuyruk, kurulum, logo, kısa + 2 uzun sorgu, zamanlanmış işler)
func telegramTur(sehirYol string) error {
	mikBirakTemizle() // v13: takılı kalmış TTS mikrofon bayrağı
	if err := bildirimGonder(); err != nil && !strings.Contains(err.Error(), "sohbet yok") {
		fmt.Fprintln(os.Stderr, "bildirim:", err)
	}
	jeton := oku(telegramJeton, "")
	if jeton == "" {
		return fmt.Errorf("telegram jetonu yok")
	}
	ctx, iptal := context.WithTimeout(context.Background(), 60*time.Second)
	defer iptal()
	sahip, err := telegramSohbetBul(ctx, jeton)
	if err != nil {
		return err
	}
	tgKurulumYap(ctx, jeton, sahip) // v11: sol menü + kalıcı düğmeler (bir kez)
	if err := tgLogoAyarla(ctx, jeton, false); err != nil {
		fmt.Fprintln(os.Stderr, "logo:", err)
	}
	ofs, _ := strconv.ParseInt(oku(telegramOffset, "0"), 10, 64)
	if _, err := tgDinleDongu(ctx, jeton, sahip, ofs, sehirYol, func(o int64) {
		_ = os.WriteFile(telegramOffset, []byte(strconv.FormatInt(o, 10)+"\n"), 0600)
	}); err != nil {
		return err
	}
	zamanlanmisIsler(ctx, jeton, sahip, sehirYol) // v9: sabah ozeti + hizmet uyarilari
	return nil
}

// tgDinleDongu v11: önce bekletmeden, sonra süre yettikçe en çok 2 uzun sorgu (her birinden önce bildirim kuyruğu).
func tgDinleDongu(ctx context.Context, jeton, sahip string, ofs int64, sehirYol string, kaydet func(int64)) (int64, error) {
	for tur := 0; tur < 3; tur++ {
		bekle := 0
		if tur > 0 {
			son, var_ := ctx.Deadline()
			if !var_ || time.Until(son) < time.Duration(tgUzunBekleme+15)*time.Second {
				break
			}
			bekle = tgUzunBekleme
			if err := bildirimGonder(); err != nil && !strings.Contains(err.Error(), "sohbet yok") {
				fmt.Fprintln(os.Stderr, "bildirim:", err)
			}
		}
		var gun []tgGuncelleme
		if err := telegramIstek(ctx, jeton, "getUpdates", url.Values{"offset": {strconv.FormatInt(ofs+1, 10)},
			"timeout": {strconv.Itoa(bekle)}, "allowed_updates": {`["message","callback_query"]`}}, &gun); err != nil {
			if tur == 0 {
				return ofs, err
			}
			return ofs, nil // uzun sorgu kesildi: sonraki turda (15 sn) yeniden
		}
		for _, u := range gun {
			if u.UpdateID > ofs {
				ofs = u.UpdateID
				kaydet(ofs)
			}
			tgGuncellemeIsle(ctx, jeton, sahip, u, sehirYol)
		}
	}
	return ofs, nil
}

func tgGuncellemeIsle(ctx context.Context, jeton, sahip string, u tgGuncelleme, sehirYol string) {
	if cq := u.CallbackQuery; cq != nil { // v11: satır içi düğme
		sohbet := cq.From.ID
		if cq.Message != nil {
			sohbet = cq.Message.Chat.ID
		}
		hedef := strconv.FormatInt(sohbet, 10) // v45: cevap düğmeye basana
		if !yetkiliSohbet(hedef, sahip) {
			return // yalnız sahip + izinliler (v45)
		}
		if k := tgGeriCagri(ctx, jeton, hedef, cq.ID, cq.Data); k != "" {
			tgKomut(ctx, jeton, hedef, k, 0, sehirYol)
		}
		return
	}
	hedef := strconv.FormatInt(u.Message.Chat.ID, 10) // v45: cevap mesajı yazana
	if !yetkiliSohbet(hedef, sahip) {
		return // yalnız sahip + izinliler (v45)
	}
	if c := fotoMesaji(ctx, jeton, u.Message.Photo, u.Message.Document, u.Message.Caption); c != "" {
		telegramYanit(ctx, jeton, hedef, c)
		return
	}
	t := strings.TrimSpace(u.Message.Text)
	if t == "" {
		return
	}
	if r := u.Message.Yanitlanan; r != nil && r.Text == jpSoruIstem && !strings.HasPrefix(t, "/") {
		t = "/jp " + t // v48: Gezi istemine verilen yanıt
	}
	tgKomut(ctx, jeton, hedef, t, u.Message.MessageID, sehirYol)
}

// tgKomut v11: menü katmanı (telegram_menu.go), sonra v10 komutları AYNEN.
func tgKomut(ctx context.Context, jeton, sahip, t string, mesajID int64, sehirYol string) {
	if d := tgDugmeKomut(t); d != "" {
		t = d // kalıcı düğme metni ("📷 Foto") -> komut ("/foto")
	}
	if tgMenuKomut(ctx, jeton, sahip, t, sehirYol) {
		return
	}
	kom := strings.Fields(strings.ToLower(t))
	arg := ""
	if len(kom) > 1 {
		arg = kom[1]
	}
	switch strings.SplitN(kom[0], "@", 2)[0] {
	case "/jp": // v46: gezi sorusu (jp.go)
		soru := strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(t), strings.Fields(t)[0]))
		if soru == "" { // v48: düğme/komut tek başına -> soruyu yanıt olarak iste
			telegramYanitDugme(ctx, jeton, sahip, jpSoruIstem, map[string]any{"force_reply": true,
				"input_field_placeholder": "Örn. 12 Kasım'da ne yapıyoruz?"})
			return
		}
		telegramYanit(ctx, jeton, sahip, jpKomut(ctx, soru))
	case "/start", "/yardim", "/help":
		telegramYanit(ctx, jeton, sahip, yardimMetni)
	case "/foto":
		if dosyaVar(kalkanBayrak) {
			telegramYanit(ctx, jeton, sahip, "Gizlilik kalkanı açık efendim, kamera kullanılamaz.")
		} else if os.WriteFile("/tmp/kamera-foto-iste", []byte("telegram\n"), 0644) == nil {
			telegramYanit(ctx, jeton, sahip, "📸 Çekiyorum efendim, birazdan gelir.")
		}
	case "/zaman": // v34: aralık ve olay tespiti (zaman_kip.go)
		telegramYanit(ctx, jeton, sahip, zamanKomut(zamanY, kom[1:]))
	case "/bekci":
		y, ad := bekciBayrak, "Bekçi modu (hareket bildirimi)"
		ac := arg == "ac" || arg == "aç"
		if arg != "ac" && arg != "aç" && arg != "kapat" {
			ac = !dosyaVar(y)
		}
		if err := bayrak(y, ac); err != nil {
			telegramYanit(ctx, jeton, sahip, "Ayarlayamadım efendim: SD kart yazılamıyor.")
		} else if ac {
			telegramYanit(ctx, jeton, sahip, ad+" açıldı efendim.")
		} else {
			telegramYanit(ctx, jeton, sahip, ad+" kapatıldı efendim.")
		}
	case "/cerceve", "/çerçeve":
		var ui map[string]any
		c := ""
		switch arg {
		case "ac", "aç":
			ui, c = map[string]any{"cerceve": true}, "Ekran koruyucu yerine fotoğraf çerçevesi açıldı efendim."
		case "kapat":
			ui, c = map[string]any{"cerceve": false}, "Fotoğraf çerçevesi kapatıldı efendim."
		default:
			ui, c = map[string]any{"ss": "cerceve"}, fmt.Sprintf("Çerçeveyi gösteriyorum efendim (%d fotoğraf).", cerceveSay(cerceveDizin))
		}
		if !panelIslem(ui) {
			c = "Panele ulaşamadım efendim."
		}
		telegramYanit(ctx, jeton, sahip, c)
	case "/durum":
		telegramYanit(ctx, jeton, sahip, durumOzeti(sehirYol)+"\n\n"+saglikOzeti())
	case "/haber", "/haberler":
		telegramYanit(ctx, jeton, sahip, haberKomutu(ctx, arg))
	case "/saglik", "/sağlık":
		telegramYanit(ctx, jeton, sahip, saglikOzeti())
	case "/ses":
		m := strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(t), strings.Fields(t)[0]))
		telegramYanit(ctx, jeton, sahip, sesKomutu(m))
	case "/sesler":
		telegramYanit(ctx, jeton, sahip, seslerKomutu(arg))
	case "/anahtar", "/anahtarlar": // v17: yedek ses servisleri; anahtarli mesaj silinir
		alan := strings.Fields(t)
		c := anahtarKomutu(alan[1:])
		if len(alan) > 2 {
			if telegramIstek(ctx, jeton, "deleteMessage", url.Values{"chat_id": {sahip},
				"message_id": {strconv.FormatInt(mesajID, 10)}}, nil) == nil {
				c += "\n🧹 Anahtarlı mesajınızı sohbetten sildim efendim."
			} else {
				c += "\n⚠️ Mesajınızı silemedim efendim, lütfen sohbetten siz silin."
			}
		}
		telegramYanit(ctx, jeton, sahip, c)
	case "/sesdurum", "/sesdurumu":
		telegramYanit(ctx, jeton, sahip, sesDurumu())
	case "/dusunme", "/düşünme": // v26
		telegramYanit(ctx, jeton, sahip, dusunmeKomutu(arg))
	case "/hiz", "/hız": // v26
		telegramYanit(ctx, jeton, sahip, hizKomutu())
	case "/heykole", "/heyköle": // v35 (heykole.go)
		telegramYanit(ctx, jeton, sahip, heyKoleKomut(arg))
	case "/seskapat", "/sesac":
		telegramYanit(ctx, jeton, sahip, sesAcKapa(kom[0] == "/sesac"))
	case "/sabah":
		telegramYanit(ctx, jeton, sahip, sabahKomutu(ctx, arg, sehirYol))
	case "/sehir", "/şehir":
		telegramYanit(ctx, jeton, sahip, sehirKomutu(ctx, strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(t), strings.Fields(t)[0])), sehirYol))
	default:
		telegramYanit(ctx, jeton, sahip, koleyeSor(t))
	}
}

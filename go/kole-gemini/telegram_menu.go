// telegram_menu.go (21.10.2026, Claude; kole-gemini v22, OTONOM Sıra 7 madde 38): Telegram'da tek dokunuşla yönetim.
// kullanıcı: "telegramdaki komutları / işaretinden sonra gelecek tek tıklama komutuna çevir tıklayarak yönetebileyim, bazıları
// telegram botlarına sola pop-up panel gibi bişey koyarak yönetiyor ... botun profil fotoğrafını bizim logo yap".
//   - Kalıcı düğme klavyesi (ReplyKeyboardMarkup, is_persistent): basılan düğmenin metni komuta çevrilir (tgDugmeKomut).
//   - Satır içi düğmeler (InlineKeyboardMarkup): aç/kapat ve seçimler; callback_query "k:<komut>" -> answerCallbackQuery +
//     komut aynen yazılmış gibi işlenir. Canlı izle / Merkez düğmeleri doğrudan bağlantıdır (url).
//   - Sol menü: setMyCommands (Türkçe açıklamalı) + setChatMenuButton {type: commands} + bot açıklamaları; sürümde bir kez
//     (/data/asistan/telegram-menu-v22; başarısızsa en erken 1 saat sonra yeniden). /menu klavyeyi her zaman gönderir.
//   - Profil fotoğrafı: setMyProfilePhoto (InputProfilePhotoStatic, multipart attach://logo) gömülü kole-logo.jpg (scripts/427
//     R_bot_640.jpg). Başarılıysa /data/asistan/telegram-logo-v1; değilse en erken 6 saat sonra; /logo zorlar.
package main

import (
	"bytes"
	"context"
	_ "embed"
	"encoding/json"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"net/textproto"
	"net/url"
	"os"
	"strconv"
	"strings"
	"time"
)

//go:embed kole-logo.jpg
var koleLogo []byte

var (
	tgMenuBayrak  = "/data/asistan/telegram-menu-v50" // v50: Gezi düğmesi en üstte; v48: "jp" komutu + 🗾 Gezi düğmesi (v35: "heykole"); liste bir kez yeniden kaydedilir
	tgMenuDeneme  = "/tmp/telegram-menu-deneme"
	tgLogoBayrak  = "/data/asistan/telegram-logo-v1"
	tgLogoDeneme  = "/tmp/telegram-logo-deneme"
	merkezAdres   = "https://velinux.example.ts.net"
	tgUzunBekleme = 20 // sn; getUpdates uzun sorgu (düğmeye basınca beklemesin)
)

type tgDugme struct{ Metin, Komut string }

// tgKlavye: alttaki kalıcı düğmeler. "#..." iç komutlar satır içi seçim menüsü gösterir.
var tgKlavye = [][]tgDugme{
	{{"🗾 Gezi", "/jp"}}, // v50: gezi süresince en üstte tek başına (kullanıcı: Gezi dönüşü geri alınacak, OTONOM)
	{{"📊 Durum", "#durum"}, {"📷 Foto", "/foto"}, {"🎥 Canlı izle", "/canli"}},
	{{"🛡 Bekçi", "#bekci"}, {"⏱ Time-lapse", "#zaman"}, {"🖼 Çerçeve", "#cerceve"}},
	{{"📰 Haberler", "#haber"}, {"☀️ Sabah özeti", "#sabah"}, {"🩺 Sağlık", "/saglik"}},
	{{"🔊 Ses", "#ses"}, {"❓ Yardım", "/yardim"}},
}

// tgKomutlar: sol menü (setMyCommands). Ad: a-z, 0-9, _ (1-32); açıklama 1-256 karakter.
var tgKomutlar = [][2]string{
	{"menu", "Düğme menüsünü göster"},
	{"durum", "Hava, kamera ve hizmetler"},
	{"foto", "Anlık fotoğraf çek"},
	{"canli", "Canlı kamera bağlantısı"},
	{"bekci", "Bekçi modu (hareket bildirimi)"},
	{"zaman", "Time-lapse (zaman atlamalı çekim)"},
	{"cerceve", "Dijital fotoğraf çerçevesi"},
	{"haber", "Haberler"},
	{"sabah", "Sabah özeti"},
	{"saglik", "Cihaz sağlığı"},
	{"ses", "Hey Köle sesi ve hoparlör"},
	{"heykole", "Hey Köle dinlemesi: aç / kapat"}, // v35
	{"sehir", "Hava durumu şehri"},
	{"jp", "gezi sorusu (site bilgisi)"}, // v48
	{"yardim", "Tüm komutlar ve örnekler"},
}

// tgDugmeKomut: kalıcı düğme metni -> komut ("" ise düğme değil).
func tgDugmeKomut(t string) string {
	t = strings.TrimSpace(t)
	for _, sat := range tgKlavye {
		for _, d := range sat {
			if t == d.Metin {
				return d.Komut
			}
		}
	}
	return ""
}

func tgKlavyeJSON() map[string]any {
	var satirlar [][]map[string]string
	for _, sat := range tgKlavye {
		var s []map[string]string
		for _, d := range sat {
			s = append(s, map[string]string{"text": d.Metin})
		}
		satirlar = append(satirlar, s)
	}
	return map[string]any{"keyboard": satirlar, "is_persistent": true, "resize_keyboard": true,
		"input_field_placeholder": "Bir düğmeye dokunun ya da yazın"}
}

// satır içi: komut düğmesi (callback "k:<komut>") ya da bağlantı (url)
func tgSatir(dugmeler ...map[string]string) []map[string]string { return dugmeler }
func tgK(metin, komut string) map[string]string {
	return map[string]string{"text": metin, "callback_data": "k:" + komut}
}
func tgU(metin, adres string) map[string]string {
	return map[string]string{"text": metin, "url": adres}
}
func tgSatirIci(satirlar ...[]map[string]string) map[string]any {
	return map[string]any{"inline_keyboard": satirlar}
}

func telegramYanitDugme(ctx context.Context, jeton, sohbet, metin string, markup any) {
	b, _ := json.Marshal(markup)
	if err := telegramIstek(ctx, jeton, "sendMessage", url.Values{"chat_id": {sohbet}, "text": {metin},
		"reply_markup": {string(b)}}, nil); err != nil {
		fmt.Fprintln(os.Stderr, "telegram dugmeli yanit:", err)
	}
}

func acikKapali(y string) string {
	if dosyaVar(y) {
		return "AÇIK ✅"
	}
	return "kapalı"
}

// tgMenuKomut: menü katmanı. true dönerse komut burada işlendi.
func tgMenuKomut(ctx context.Context, jeton, sahip, t, sehirYol string) bool {
	k := t
	if d := tgDugmeKomut(t); d != "" {
		k = d
	}
	alan := strings.Fields(strings.ToLower(k))
	if len(alan) == 0 {
		return false
	}
	bas := strings.SplitN(alan[0], "@", 2)[0]
	canliSatir := tgSatir(tgK("🎥 Canlı izle", "/canli"), tgU("🏠 KÖLE Merkez", merkezAdres+"/")) // v30: Telegram içinde
	switch bas {
	case "/menu", "/start":
		telegramYanitDugme(ctx, jeton, sahip, "🤖 KÖLE menüsü hazır efendim. Alttaki düğmelerle ya da soldaki "+
			"Menü'den tek dokunuşla yönetebilirsiniz; normal yazarsanız da anlarım.", tgKlavyeJSON())
	case "#durum":
		telegramYanitDugme(ctx, jeton, sahip, durumOzeti(sehirYol)+"\n\n"+saglikOzeti(), tgSatirIci(
			tgSatir(tgK("📷 Foto", "/foto"), tgK("🩺 Sağlık", "/saglik")), canliSatir))
	case "/canli", "/canlı":
		arg := ""
		if len(alan) > 1 {
			arg = alan[1]
		}
		tgCanli(ctx, jeton, sahip, arg) // v30: Telegram içinde canlı (canli_tg.go); uzat / dur da buradan
	case "#zaman": // v34: durum + aralık + olay düğmeleri (zaman_kip.go)
		telegramYanitDugme(ctx, jeton, sahip, zamanDurumMetni(zamanY), zamanDugmeleri(zamanY))
	case "#bekci":
		y, ad, kom := bekciBayrak, "🛡 Bekçi modu (hareket görünce fotoğraflı bildirim)", "/bekci"
		telegramYanitDugme(ctx, jeton, sahip, ad+": "+acikKapali(y), tgSatirIci(
			tgSatir(tgK("✅ Aç", kom+" ac"), tgK("⏹ Kapat", kom+" kapat"))))
	case "#cerceve":
		telegramYanitDugme(ctx, jeton, sahip, fmt.Sprintf("🖼 Dijital çerçeve: %d fotoğraf. Bana fotoğraf gönderirseniz "+
			"eklerim.", cerceveSay(cerceveDizin)), tgSatirIci(
			tgSatir(tgK("▶️ Şimdi göster", "/cerceve goster")),
			tgSatir(tgK("✅ Koruyucu olarak aç", "/cerceve ac"), tgK("⏹ Kapat", "/cerceve kapat"))))
	case "#haber":
		telegramYanitDugme(ctx, jeton, sahip, "📰 Hangi haberler efendim?", tgSatirIci(
			tgSatir(tgK("⭐ Öne çıkanlar", "/haber")),
			tgSatir(tgK("🏛 Gündem", "/haber siyaset"), tgK("💹 Ekonomi", "/haber ekonomi")),
			tgSatir(tgK("💻 Teknoloji", "/haber teknoloji"), tgK("🌍 Dünya", "/haber dunya")),
			tgSatir(tgK("🛡 Siber güvenlik", "/haber siber")))) // v29
	case "#sabah":
		telegramYanitDugme(ctx, jeton, sahip, "☀️ Sabah özeti (hava, haberler, döviz, hatırlatıcılar). Saati değiştirmek "+
			"için yazın: /sabah 07:30", tgSatirIci(
			tgSatir(tgK("📨 Şimdi gönder", "/sabah")),
			tgSatir(tgK("✅ Aç", "/sabah ac"), tgK("⏹ Kapat", "/sabah kapat"))))
	case "#ses":
		telegramYanitDugme(ctx, jeton, sahip, "🔊 Hey Köle sesi. Hoparlörden söyletmek için yazın: /ses merhaba",
			tgSatirIci(
				tgSatir(tgK("🔊 Sesli cevap aç", "/sesac"), tgK("🔇 Sesli cevap kapat", "/seskapat")),
				tgSatir(tgK("📋 Ses durumu", "/sesdurum"), tgK("🎙 Sesler", "/sesler")),
				tgSatir(tgK("🎤 Hey Köle aç", "/heykole ac"), tgK("🔇 Hey Köle kapat", "/heykole kapat")))) // v35
	case "#heykole": // v35
		telegramYanitDugme(ctx, jeton, sahip, heyKoleDurum(), tgSatirIci(
			tgSatir(tgK("🎤 Aç", "/heykole ac"), tgK("🔇 Kapat", "/heykole kapat"))))
	case "/logo":
		_ = os.Remove(tgLogoDeneme)
		if err := tgLogoAyarla(ctx, jeton, true); err != nil {
			telegramYanit(ctx, jeton, sahip, "Profil fotoğrafını ayarlayamadım efendim: "+err.Error())
		} else {
			telegramYanit(ctx, jeton, sahip, "🤖 Profil fotoğrafım KÖLE logosu oldu efendim.")
		}
	default:
		if strings.HasPrefix(bas, "#") {
			return true // bilinmeyen iç komut: sessizce yut
		}
		if len(alan) == 1 && (bas == "/bekci" || bas == "/zaman") { // v22: argümansız -> durum + düğmeler
			return tgMenuKomut(ctx, jeton, sahip, "#"+strings.TrimPrefix(bas, "/"), sehirYol)
		}
		if len(alan) == 1 && (bas == "/cerceve" || bas == "/çerçeve") {
			return tgMenuKomut(ctx, jeton, sahip, "#cerceve", sehirYol)
		}
		if len(alan) == 1 && (bas == "/heykole" || bas == "/heyköle") { // v35: argümansız -> durum + düğmeler
			return tgMenuKomut(ctx, jeton, sahip, "#heykole", sehirYol)
		}
		if len(alan) == 1 && bas == "/ses" {
			return tgMenuKomut(ctx, jeton, sahip, "#ses", sehirYol)
		}
		return false
	}
	return true
}

func zamanDenemesiYeni(y string, sure time.Duration) bool {
	b, err := os.ReadFile(y)
	if err != nil {
		return false
	}
	t, err := strconv.ParseInt(strings.TrimSpace(string(b)), 10, 64)
	return err == nil && time.Since(time.Unix(t, 0)) < sure
}

func zamanDenemesiYaz(y string) {
	_ = os.WriteFile(y, []byte(strconv.FormatInt(time.Now().Unix(), 10)+"\n"), 0600)
}

// tgKurulumYap: sol menü + açıklamalar + kalıcı klavye (sürümde bir kez).
func tgKurulumYap(ctx context.Context, jeton, sahip string) {
	if dosyaVar(tgMenuBayrak) || zamanDenemesiYeni(tgMenuDeneme, time.Hour) {
		return
	}
	zamanDenemesiYaz(tgMenuDeneme)
	var komutlar []map[string]string
	for _, k := range tgKomutlar {
		komutlar = append(komutlar, map[string]string{"command": k[0], "description": k[1]})
	}
	kb, _ := json.Marshal(komutlar)
	hata := 0
	if err := telegramIstek(ctx, jeton, "setMyCommands", url.Values{"commands": {string(kb)}}, nil); err != nil {
		fmt.Fprintln(os.Stderr, "setMyCommands:", err)
		hata++
	}
	if err := telegramIstek(ctx, jeton, "setChatMenuButton", url.Values{"chat_id": {sahip},
		"menu_button": {`{"type":"commands"}`}}, nil); err != nil {
		fmt.Fprintln(os.Stderr, "setChatMenuButton:", err)
	}
	_ = telegramIstek(ctx, jeton, "setMyShortDescription", url.Values{"short_description": {
		"VeLinux KÖLE: evinizdeki panelin asistanı. Kamera, haberler, hatırlatıcılar."}}, nil)
	_ = telegramIstek(ctx, jeton, "setMyDescription", url.Values{"description": {
		"🤖 VeLinux KÖLE — emrinizdeyim efendim. Alttaki düğmelerle ya da soldaki Menü'den tek dokunuşla kamera, " +
			"bekçi, time-lapse, çerçeve, haberler ve sabah özetini yönetin; normal yazarak da soru sorabilirsiniz."}}, nil)
	b, _ := json.Marshal(tgKlavyeJSON())
	if err := telegramIstek(ctx, jeton, "sendMessage", url.Values{"chat_id": {sahip}, "text": {
		"🤖 Yeni menü hazır efendim: alttaki düğmelerle ya da soldaki Menü'den tek dokunuşla yönetebilirsiniz."},
		"reply_markup": {string(b)}}, nil); err != nil {
		fmt.Fprintln(os.Stderr, "menu klavyesi:", err)
		hata++
	}
	if hata == 0 {
		_ = os.WriteFile(tgMenuBayrak, []byte(time.Now().Format(time.RFC3339)+"\n"), 0600)
	}
}

// tgLogoAyarla: bot profil fotoğrafı = gömülü KÖLE logosu (bir kez; zorla=true her durumda dener).
func tgLogoAyarla(ctx context.Context, jeton string, zorla bool) error {
	if !zorla && (dosyaVar(tgLogoBayrak) || zamanDenemesiYeni(tgLogoDeneme, 6*time.Hour)) {
		return nil
	}
	zamanDenemesiYaz(tgLogoDeneme)
	var g bytes.Buffer
	w := multipart.NewWriter(&g)
	_ = w.WriteField("photo", `{"type":"static","photo":"attach://logo"}`)
	h := make(textproto.MIMEHeader)
	h.Set("Content-Disposition", `form-data; name="logo"; filename="kole-logo.jpg"`)
	h.Set("Content-Type", "image/jpeg")
	p, _ := w.CreatePart(h)
	_, _ = p.Write(koleLogo)
	_ = w.Close()
	req, _ := http.NewRequestWithContext(ctx, http.MethodPost, tgTaban+"/bot"+jeton+"/setMyProfilePhoto", &g)
	req.Header.Set("Content-Type", w.FormDataContentType())
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<16))
	var y struct {
		Ok          bool   `json:"ok"`
		Description string `json:"description"`
	}
	if json.Unmarshal(b, &y) != nil || !y.Ok {
		return fmt.Errorf("setMyProfilePhoto: HTTP %d %s", res.StatusCode, y.Description)
	}
	return os.WriteFile(tgLogoBayrak, []byte(time.Now().Format(time.RFC3339)+"\n"), 0600)
}

// tgGeriCagri: satır içi düğme. Döner: yürütülecek komut ("" ise yok).
func tgGeriCagri(ctx context.Context, jeton, sahip string, id, veri string) string {
	_ = telegramIstek(ctx, jeton, "answerCallbackQuery", url.Values{"callback_query_id": {id}}, nil)
	if !strings.HasPrefix(veri, "k:") {
		return ""
	}
	return strings.TrimSpace(veri[2:])
}

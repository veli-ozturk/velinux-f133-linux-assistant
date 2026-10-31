// saglayici.go (20.10.2026, Claude): v17 ÜCRETSİZ SAĞLAYICI ROTASYONU + SOĞUMA HAFIZASI (scripts/379).
// kullanıcı: "başka nereden ücretsiz api bulabiliriz? onları da kendi içinde dönecek şekilde entegre edebilir miyiz".
// TTS adayları sırayla: Gemini TTS modelleri (aynı anahtar, model başına ayrı kota, aynı Puck sesi) -> Google Cloud TTS
// (Chirp3-HD; kendi anahtarı ya da Gemini anahtarıyla yoklama) -> Azure Speech F0 -> ElevenLabs (anahtar dosyası varsa).
// 429 alan aday soğumaya girer (retryDelay / günlük kota: Pasifik gece yarısı / 30 dk) ve sona alınır; 401/403/404 24 sa.
package main

import (
	"bytes"
	"context"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"encoding/xml"
	"errors"
	"fmt"
	"io"
	"net/http"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"
)

var (
	sogumaYol          = "/tmp/api-soguma.json"
	gcloudAnahtarYol   = "/data/asistan/anahtar-gcloud"
	gcloudSesYol       = "/data/asistan/gcloud-ses"
	azureAnahtarYol    = "/data/asistan/anahtar-azure"
	azureSesYol        = "/data/asistan/azure-ses"
	elevenAnahtarYol   = "/data/asistan/anahtar-eleven"
	ttsModellerYol     = "/data/asistan/tts-modeller" // isteğe bağlı ek Gemini TTS modelleri (boşlukla/satırla)
	gcloudURL          = "https://texttospeech.googleapis.com/v1/text:synthesize"
	azureURLKalip      = "https://%s.tts.speech.microsoft.com/cognitiveservices/v1"
	elevenURLKalip     = "https://api.elevenlabs.io/v1/text-to-speech/%s?output_format=pcm_16000"
	ttsGeminiModelleri = []string{"gemini-3.8-flash-tts", "gemini-3.8-flash-lite-tts", "gemini-3.1-flash-tts-preview",
		"gemini-2.5-pro-preview-tts"}
)

const (
	azureSesVarsayilan  = "tr-TR-AhmetNeural"
	elevenSesVarsayilan = "TX3LPaxmHKxFdv7VOQHJ" // v38: premade Liam (genç erkek); Josh (TxGEqn..) kütüphane sesi -> ücretsizde 402
	elevenModel         = "eleven_flash_v2_5"
)

// ---------------------------------------------------------------- soğuma hafızası (/tmp: açılışta sıfırlanır)

type soguma struct {
	Kadar int64  `json:"kadar"`
	Neden string `json:"neden"`
}

func sogumaOku() map[string]soguma {
	m := map[string]soguma{}
	if b, err := os.ReadFile(sogumaYol); err == nil {
		_ = json.Unmarshal(b, &m)
	}
	return m
}

func sogumaYaz(m map[string]soguma) {
	now := time.Now().Unix()
	for k, v := range m {
		if v.Kadar <= now {
			delete(m, k)
		}
	}
	if b, err := json.Marshal(m); err == nil {
		_ = dosyaYazAtomik(sogumaYol, b, 0644)
	}
}

func sogumaKoy(ad string, sure time.Duration, neden string) {
	m := sogumaOku()
	m[ad] = soguma{time.Now().Add(sure).Unix(), neden}
	sogumaYaz(m)
}

func sogumaKaldir(ad string) {
	m := sogumaOku()
	if _, ok := m[ad]; ok {
		delete(m, ad)
		sogumaYaz(m)
	}
}

func sogumaDurum(ad string) (soguma, bool) {
	v, ok := sogumaOku()[ad]
	return v, ok && v.Kadar > time.Now().Unix()
}

// 429: retryDelay / Retry-After > günlük kota (kota adında PerDay) -> Pasifik gece yarısı > 30 dk
func sogumaKoyKota(ad string, govde []byte, retryAfter string) time.Duration {
	kota, _, bekle, _ := kotaCoz(govde, retryAfter)
	sure := 30 * time.Minute
	switch {
	case bekle > 0:
		sure = time.Duration(bekle) * time.Second
	case strings.Contains(kota, "PerDay"):
		sure = time.Until(pasifikGeceYarisi(time.Now())) + time.Minute
	}
	sogumaKoy(ad, sure, "429")
	return sure
}

// hazırlar önce; 429 soğumasındakiler sonda (son çare); 401/403/404/400 olanlar yalnız başka aday yoksa
func sogumaSirala(adlar []string, onek string) []string {
	m := sogumaOku()
	now := time.Now().Unix()
	var hazir, kota, kapali []string
	for _, a := range adlar {
		a = strings.TrimSpace(a)
		if a == "" {
			continue
		}
		v, ok := m[onek+a]
		switch {
		case !ok || v.Kadar <= now:
			hazir = append(hazir, a)
		case v.Neden == "429":
			kota = append(kota, a)
		default:
			kapali = append(kapali, a)
		}
	}
	o := append(hazir, kota...)
	if len(o) == 0 {
		o = kapali
	}
	return o
}

// Google RPD kotaları Pasifik gece yarısı sıfırlanır (TR yazın 10:00, kışın 11:00). ABD yaz saati: Mart 2. pazar -
// Kasım 1. pazar, 02:00 yerel.
func pasifikGeceYarisi(t time.Time) time.Time {
	u := t.UTC()
	off := -8
	if abdYazSaati(u) {
		off = -7
	}
	loc := time.FixedZone("PT", off*3600)
	p := u.In(loc)
	return time.Date(p.Year(), p.Month(), p.Day()+1, 0, 0, 0, 0, loc)
}

func abdYazSaati(u time.Time) bool {
	pazar := func(ay time.Month, n int) time.Time {
		t := time.Date(u.Year(), ay, 1, 0, 0, 0, 0, time.UTC)
		for t.Weekday() != time.Sunday {
			t = t.AddDate(0, 0, 1)
		}
		return t.AddDate(0, 0, 7*(n-1))
	}
	bas := pazar(time.March, 2).Add(10 * time.Hour)   // 02:00 PST
	bit := pazar(time.November, 1).Add(9 * time.Hour) // 02:00 PDT
	return !u.Before(bas) && u.Before(bit)
}

// ---------------------------------------------------------------- adaylar

type ttsDeneme struct {
	ad, etiket string
	dene       func(ctx context.Context) (x []int16, hz, kod int, govde []byte, retryAfter string, err error)
}

func httpGonder(ctx context.Context, adres string, baslik map[string]string, govde []byte) (int, []byte, string, error) {
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, adres, bytes.NewReader(govde))
	if err != nil {
		return 0, nil, "", err
	}
	for k, v := range baslik {
		req.Header.Set(k, v)
	}
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return 0, nil, "", err
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 24<<20))
	return res.StatusCode, b, res.Header.Get("Retry-After"), nil
}

func pcmBayt16(b []byte) []int16 {
	x := make([]int16, len(b)/2)
	for i := range x {
		x[i] = int16(binary.LittleEndian.Uint16(b[2*i:]))
	}
	return x
}

func xmlKacis(s string) string {
	var b bytes.Buffer
	_ = xml.EscapeText(&b, []byte(s))
	return b.String()
}

// stil boşsa annotations yok (eski modeller speech_metadata kabul etmeyebilir)
func ttsGeminiGovde(model, metin, ses, stil string) []byte {
	if stil != "" {
		return ttsGovde(model, metin, ses, stil)
	}
	g := map[string]any{
		"model":             model,
		"input":             []any{map[string]any{"type": "user_input", "content": []any{map[string]any{"type": "text", "text": metin}}}},
		"response_format":   map[string]any{"type": "audio", "mime_type": "audio/l16", "sample_rate": 24000},
		"generation_config": map[string]any{"speech_config": []any{map[string]any{"voice": ses}}},
	}
	b, _ := json.Marshal(g)
	return b
}

func ttsGeminiListesi() []string {
	l := []string{oku(ttsModelYol, ttsModelVarsayilan)}
	if b, err := os.ReadFile(ttsModellerYol); err == nil {
		l = append(l, strings.Fields(string(b))...)
	}
	l = append(l, ttsGeminiModelleri...)
	gor := map[string]bool{}
	var o []string
	for _, m := range l {
		if m != "" && !gor[m] {
			gor[m] = true
			o = append(o, m)
		}
	}
	return o
}

var stilHataRe = regexp.MustCompile(`(?i)annotation|speech_metadata|style`)

func ttsAdaylar(anahtar, metin, ses, stil string, yalnizGemini bool) []ttsDeneme {
	var a []ttsDeneme
	for i, m := range ttsGeminiListesi() {
		m := m
		et := ses
		if i > 0 {
			et = ses + " (" + strings.TrimPrefix(m, "gemini-") + ")"
		}
		a = append(a, ttsDeneme{"gemini:" + m, et, func(ctx context.Context) ([]int16, int, int, []byte, string, error) {
			stilli := stil != ""
			if _, ok := sogumaDurum("tts-stilsiz:" + m); ok {
				stilli = false
			}
			for {
				st := stil
				if !stilli {
					st = ""
				}
				kod, b, ra, err := httpGonder(ctx, ttsURL, map[string]string{"Content-Type": "application/json",
					"x-goog-api-key": anahtar}, ttsGeminiGovde(m, metin, ses, st))
				if err != nil {
					return nil, 0, 0, nil, "", err
				}
				if kod == 200 {
					x, hz, err := ttsSesCoz(b)
					return x, hz, kod, nil, ra, err
				}
				if kod == 400 && stilli && stilHataRe.Match(b) {
					stilli = false
					sogumaKoy("tts-stilsiz:"+m, 7*24*time.Hour, "stil")
					continue
				}
				return nil, 0, kod, b, ra, nil
			}
		}})
	}
	if yalnizGemini {
		return a
	}
	gk, gad := oku(gcloudAnahtarYol, ""), "gcloud"
	if gk == "" { // kendi anahtarı yoksa Gemini anahtarıyla yokla (Cloud TTS projede açık değilse 403 -> 24 sa)
		gk, gad = anahtar, "gcloud-gemini-anahtari"
	}
	if gk != "" {
		gses := oku(gcloudSesYol, "tr-TR-Chirp3-HD-"+ses)
		a = append(a, ttsDeneme{gad, "GCloud " + gses, func(ctx context.Context) ([]int16, int, int, []byte, string, error) {
			var kod int
			var b []byte
			var ra string
			for _, v := range []string{gses, "tr-TR-Wavenet-B"} {
				g, _ := json.Marshal(map[string]any{"input": map[string]any{"text": metin},
					"voice":       map[string]any{"languageCode": "tr-TR", "name": v},
					"audioConfig": map[string]any{"audioEncoding": "LINEAR16", "sampleRateHertz": 16000}})
				var err error
				kod, b, ra, err = httpGonder(ctx, gcloudURL, map[string]string{"Content-Type": "application/json",
					"x-goog-api-key": gk}, g)
				if err != nil {
					return nil, 0, 0, nil, "", err
				}
				if kod == 200 {
					var y struct {
						AudioContent string `json:"audioContent"`
					}
					if json.Unmarshal(b, &y) != nil || y.AudioContent == "" {
						return nil, 0, kod, nil, ra, fmt.Errorf("gcloud: yanıtta ses yok")
					}
					ham, err := base64.StdEncoding.DecodeString(y.AudioContent)
					if err != nil {
						return nil, 0, kod, nil, ra, fmt.Errorf("gcloud: ses base64 değil")
					}
					if bytes.HasPrefix(ham, []byte("RIFF")) {
						x, hz, err := wavCoz(ham)
						return x, hz, kod, nil, ra, err
					}
					return pcmBayt16(ham), 16000, kod, nil, ra, nil
				}
				if kod != 400 { // 400: ses adı yoksa yedek sesle bir kez daha
					break
				}
			}
			return nil, 0, kod, b, ra, nil
		}})
	}
	if f := strings.Fields(oku(azureAnahtarYol, "")); len(f) == 2 {
		bolge, ak := f[0], f[1]
		v := oku(azureSesYol, azureSesVarsayilan)
		a = append(a, ttsDeneme{"azure", "Azure " + v, func(ctx context.Context) ([]int16, int, int, []byte, string, error) {
			ssml := "<speak version='1.0' xml:lang='tr-TR'><voice name='" + xmlKacis(v) + "'>" + xmlKacis(metin) +
				"</voice></speak>"
			kod, b, ra, err := httpGonder(ctx, fmt.Sprintf(azureURLKalip, bolge), map[string]string{
				"Ocp-Apim-Subscription-Key": ak, "Content-Type": "application/ssml+xml",
				"X-Microsoft-OutputFormat": "raw-16khz-16bit-mono-pcm", "User-Agent": "velinux-kole"}, []byte(ssml))
			if err != nil {
				return nil, 0, 0, nil, "", err
			}
			if kod == 200 {
				if len(b) < 320 {
					return nil, 0, kod, nil, ra, fmt.Errorf("azure: ses çok kısa")
				}
				return pcmBayt16(b), 16000, kod, nil, ra, nil
			}
			return nil, 0, kod, b, ra, nil
		}})
	}
	if f := strings.Fields(oku(elevenAnahtarYol, "")); len(f) >= 1 {
		ek, sid := f[0], elevenSesVarsayilan
		if len(f) > 1 {
			sid = f[1]
		}
		a = append(a, ttsDeneme{"eleven", "ElevenLabs", func(ctx context.Context) ([]int16, int, int, []byte, string, error) {
			g, _ := json.Marshal(map[string]any{"text": metin, "model_id": elevenModel})
			kod, b, ra, err := httpGonder(ctx, fmt.Sprintf(elevenURLKalip, sid), map[string]string{"xi-api-key": ek,
				"Content-Type": "application/json", "Accept": "audio/pcm"}, g)
			if err != nil {
				return nil, 0, 0, nil, "", err
			}
			if kod == 200 {
				if len(b) < 320 {
					return nil, 0, kod, nil, ra, fmt.Errorf("eleven: ses çok kısa")
				}
				return pcmBayt16(b), 16000, kod, nil, ra, nil
			}
			return nil, 0, kod, b, ra, nil
		}})
	}
	return a
}

// günlüğe/hataya hiçbir anahtar yazılmaz
func gizle(s string, anahtar string) string {
	for _, k := range []string{anahtar, oku(gcloudAnahtarYol, "")} {
		if k != "" {
			s = strings.ReplaceAll(s, k, "***")
		}
	}
	for _, y := range []string{azureAnahtarYol, elevenAnahtarYol} {
		for _, k := range strings.Fields(oku(y, "")) {
			if len(k) >= 12 {
				s = strings.ReplaceAll(s, k, "***")
			}
		}
	}
	return s
}

func ttsUretGemini(ctx context.Context, anahtar, metin string) ([]int16, string, error) {
	return ttsUretSec(ctx, anahtar, metin, true)
}

// v42 (denetim v2 #11): her sağlayıcı denemesine kendi süresi; takılan biri kalan bütçenin hepsini yemesin
var ttsDenemeSure = 10 * time.Second

// uzun metin daha uzun sürer: Gemini üretimi ~65 ms/karakter ölçüldü (108 karakter 7040 ms, 20.10); pay ile 90 ms
var ttsDenemeKarakter = 90 * time.Millisecond

// ttsKirp: TTS'e en çok 700 karakter (tam yol; v42'den beri akış da)
func ttsKirp(metin string) string {
	if r := []rune(metin); len(r) > 700 {
		return string(r[:700])
	}
	return metin
}

// adaylar soğuma sırasıyla denenir; ilk başarılı ses 16 kHz'e çevrilir. etiket: ses/sağlayıcı (günlük için)
func ttsUretSec(ctx context.Context, anahtar, metin string, yalnizGemini bool) ([]int16, string, error) {
	metin = ttsKirp(metin)
	ses := ttsSesAdi()
	adaylar := ttsAdaylar(anahtar, metin, ses, oku(ttsStilYol, ttsStilVarsayilan), yalnizGemini)
	adlar := make([]string, len(adaylar))
	idx := map[string]ttsDeneme{}
	for i, a := range adaylar {
		adlar[i] = a.ad
		idx[a.ad] = a
	}
	son := errors.New("ses sağlayıcısı yok")
	for _, ad := range sogumaSirala(adlar, "tts:") {
		a := idx[ad]
		dctx, diptal := context.WithTimeout(ctx, ttsDenemeSure+time.Duration(len([]rune(metin)))*ttsDenemeKarakter) // v42
		x, hz, kod, b, ra, err := a.dene(dctx)
		diptal()
		if err == nil && kod == 200 && len(x) > 0 {
			sogumaKaldir("tts:" + ad)
			y := yenidenOrnekle(x, hz, ttsHedefHz)
			if len(y) > ttsAzamiSn*ttsHedefHz {
				y = y[:ttsAzamiSn*ttsHedefHz]
			}
			return y, a.etiket, nil
		}
		if err != nil || kod == 200 {
			if err == nil {
				err = errors.New("boş ses")
			}
			son = errors.New(gizle(ad+": "+err.Error(), anahtar))
			hazirGunluk("tts sağlayıcı: %s hata: %s", ad, son) // v38: sessiz geçmesin
			if ctx.Err() != nil {
				return nil, ses, son
			}
			continue
		}
		temiz := gizle(string(b), anahtar)
		msg := temiz
		if len(msg) > 160 {
			msg = msg[:160]
		}
		son = fmt.Errorf("%s: HTTP %d %s", ad, kod, msg)
		switch kod {
		case 429:
			kotaKaydet(strings.TrimPrefix(ad, "gemini:"), []byte(temiz), ra)
			d := sogumaKoyKota("tts:"+ad, []byte(temiz), ra)
			hazirGunluk("tts sağlayıcı: %s 429, %d dk sona alındı", ad, int(d.Minutes()+0.5))
		case 401, 403, 404:
			sogumaKoy("tts:"+ad, 24*time.Hour, strconv.Itoa(kod))
			hazirGunluk("tts sağlayıcı: %s HTTP %d, 24 sa devre dışı", ad, kod)
		case 400:
			sogumaKoy("tts:"+ad, 6*time.Hour, "400")
			hazirGunluk("tts sağlayıcı: %s HTTP 400, 6 sa devre dışı: %s", ad, msg)
		case 402: // v38: ElevenLabs ücretsizde kütüphane sesi (paid_plan_required)
			sogumaKoy("tts:"+ad, 24*time.Hour, "402")
			hazirGunluk("tts sağlayıcı: %s HTTP 402 (ücretli plan gerekiyor), 24 sa devre dışı: %s", ad, msg)
		default: // v38: görünür olsun
			hazirGunluk("tts sağlayıcı: %s HTTP %d: %s", ad, kod, msg)
		}
	}
	return nil, ses, son
}

// /sesdurum eki
func saglayiciDurumu() string {
	var b strings.Builder
	b.WriteString("\nSes sağlayıcıları (sırayla):\n")
	for _, a := range ttsAdaylar("x", "", ttsSesAdi(), "", false) {
		if v, ok := sogumaDurum("tts:" + a.ad); ok {
			fmt.Fprintf(&b, "○ %s — %s, %s'e kadar sonda\n", a.ad, v.Neden, time.Unix(v.Kadar, 0).In(trZaman).Format("15:04"))
		} else {
			fmt.Fprintf(&b, "● %s — hazır\n", a.ad)
		}
	}
	for _, s := range [][2]string{{azureAnahtarYol, "Azure (ayda ~500 bin karakter ücretsiz): /anahtar azure <bölge> <anahtar>"},
		{gcloudAnahtarYol, "Google Cloud TTS kendi anahtarı: /anahtar gcloud <anahtar>"},
		{elevenAnahtarYol, "ElevenLabs (ayda 10 bin karakter): /anahtar eleven <anahtar>"}} {
		if !dosyaVar(s[0]) {
			b.WriteString("– " + s[1] + "\n")
		}
	}
	return b.String()
}

// ---------------------------------------------------------------- Telegram /anahtar

var (
	bolgeRe   = regexp.MustCompile(`^[a-z0-9]{3,30}$`)
	anahtarRe = regexp.MustCompile(`^[A-Za-z0-9_.\-]{16,200}$`)
	sesIDRe   = regexp.MustCompile(`^[A-Za-z0-9]{10,40}$`)
)

func anahtarKomutu(a []string) string {
	yollar := map[string]string{"azure": azureAnahtarYol, "gcloud": gcloudAnahtarYol, "eleven": elevenAnahtarYol,
		"assembly": assemblyAnahtarYol} // v23
	kullanim := "🔑 Yedek ses servisleri (Gemini kotası dolunca sırayla):\n" +
		"/anahtar azure <bölge> <anahtar> — ör. /anahtar azure westeurope 1a2b...\n" +
		"/anahtar gcloud <anahtar> — Google Cloud Text-to-Speech\n" +
		"/anahtar eleven <anahtar> [ses_id] — ElevenLabs\n" +
		"/anahtar assembly <anahtar> — AssemblyAI: Gemini sesi anlayamazsa konuşmayı metne çevirir\n" +
		"/anahtar sil <servis>\nKayıtlı: "
	if len(a) == 0 {
		var k []string
		for _, s := range []string{"azure", "gcloud", "eleven", "assembly"} {
			if dosyaVar(yollar[s]) {
				k = append(k, s)
			}
		}
		if len(k) == 0 {
			return kullanim + "yok."
		}
		return kullanim + strings.Join(k, ", ") + "."
	}
	servis := strings.ToLower(a[0])
	if servis == "sil" && len(a) == 2 {
		y, ok := yollar[strings.ToLower(a[1])]
		if !ok {
			return "Bu servisi tanımıyorum efendim (azure, gcloud, eleven, assembly)."
		}
		_ = os.Remove(y)
		sogumaKaldir("tts:" + strings.ToLower(a[1]))
		return "🗑 " + strings.ToLower(a[1]) + " anahtarı silindi efendim."
	}
	y, ok := yollar[servis]
	if !ok {
		return kullanim + "?"
	}
	var icerik, anahtar string
	switch {
	case servis == "azure" && len(a) == 3 && bolgeRe.MatchString(strings.ToLower(a[1])) && anahtarRe.MatchString(a[2]):
		anahtar, icerik = a[2], strings.ToLower(a[1])+" "+a[2]
	case servis == "gcloud" && len(a) == 2 && anahtarRe.MatchString(a[1]):
		anahtar, icerik = a[1], a[1]
	case servis == "eleven" && (len(a) == 2 || len(a) == 3) && anahtarRe.MatchString(a[1]) &&
		(len(a) == 2 || sesIDRe.MatchString(a[2])):
		anahtar, icerik = a[1], strings.Join(a[1:], " ")
	case servis == "assembly" && len(a) == 2 && anahtarRe.MatchString(a[1]): // v23
		anahtar, icerik = a[1], a[1]
	default:
		return "Biçim uymadı efendim, hiçbir şey kaydetmedim.\n" + kullanim + "?"
	}
	if err := dosyaYazAtomik(y, []byte(icerik+"\n"), 0600); err != nil {
		return "Anahtarı kaydedemedim efendim."
	}
	_ = os.Chmod(y, 0600)
	if servis == "assembly" { // v23
		return fmt.Sprintf("🔑 assembly anahtarı kaydedildi efendim (…%s). Gemini sesinizi anlayamazsa konuşmayı metne "+
			"çevirip yine cevap veririm.", anahtar[len(anahtar)-4:])
	}
	sogumaKaldir("tts:" + servis)
	return fmt.Sprintf("🔑 %s anahtarı kaydedildi efendim (…%s). Gemini kotası dolunca sıraya girer; /sesdurum ile görün.",
		servis, anahtar[len(anahtar)-4:])
}

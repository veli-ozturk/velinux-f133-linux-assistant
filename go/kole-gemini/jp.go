package main

// jp.go v49 (31.10.2026, Claude): v48 + hatırlatma durum etiketleri (✅ YAPILDI / ☑️ GEREK KALMADI / ⏳ YAPILACAK) talimatta,
// önbellek 15 dk (bot ayırtılmış oteli "ayırt" diye önermişti).
// jp.go v48 (30.10.2026, Claude): v47 + 🗾 Gezi düğmesi (force_reply istemi) ve cevabın sonunda "🔗 Sayfa: <adres>"
// (bilgi.txt'teki SAYFA satırından; yoksa ana sayfa).
// jp.go v47 (30.10.2026, Claude): v46 + cevabın ilk satırı jpEtiket.
// jp.go v46 (30.10.2026, Claude; kullanıcı: "/jp <soru>" ile gezi soruları): example.pages.dev/bilgi.txt
// (~97 KB düz metin; rezervasyon kodu/PIN/pasaport yok) Cookie jp26=<anahtar> ile çekilir, SD'de 1 sa önbelleklenir;
// Gemini yalnız bu belgeye dayanarak kısa Türkçe cevap verir. Çekilemezse son önbellek kullanılır ve belirtilir.
// Anahtar /data/asistan/jp26-anahtar (0600); günlüğe ya da hata metnine yazılmaz (URL'de değil, çerezde).

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
	"unicode/utf8"
)

var (
	jpBilgiURL    = "https://example.pages.dev/bilgi.txt"
	jpAnahtarYol  = "/data/asistan/jp26-anahtar"
	jpOnbellekYol = "/mnt/sd/velinux/asistan/jp-bilgi.txt" // SD: /tmp RAM'de
)

const (
	jpTaze  = 15 * time.Minute // v49: sitedeki tikler (YAPILDI/YAPILACAK) çabuk yansısın (v46-v48: 1 sa)
	jpAzami = 4000             // Telegram mesaj sınırı payı (karakter)
	jpBoyut = 512 << 10
)

// v47: cevabı kimin verdiği belli olsun (Gezi oturumundaki Claude "🟣 Claude ·" ile yazar)
const jpEtiket = "🤖 Gemini · site bilgisi"

// v48: 🗾 Gezi düğmesi bu istemi force_reply ile gönderir; buna verilen yanıt /jp sorusu sayılır
const jpSoruIstem = "🗾 Geziyle ilgili sorunuzu bu mesaja yanıt olarak yazın efendim:"

const jpTalimat = "Sen kullanıcının ev asistanı Köle'sin. Aşağıdaki BELGE, gezinin bilgi paketidir. " +
	"Soruyu YALNIZ bu belgeye dayanarak Türkçe, kısa ve net cevapla (gerekirse madde işaretleri). Belgede yoksa " +
	"\"Bu sitede yok efendim.\" de; tahmin etme. Saatler Gezi saatidir (JST). Düz metin yaz, Markdown başlığı kullanma. " +
	"Son satıra, cevabı aldığın bölümün altındaki \"SAYFA:\" satırındaki adresi \"🔗 Sayfa: <adres>\" olarak yaz " + // v48
	"(birden çok bölümse en ilgili olanı). Belgede SAYFA satırı yoksa son satır \"🔗 Sayfa: " + jpAnaSayfa + "\" olsun; adres uydurma. " +
	"Hatırlatmalarda \"✅ YAPILDI\" ya da \"☑️ GEREK KALMADI\" yazanları yapılacak iş olarak önerme; sorulursa yapıldığını " + // v49
	"söyle. Yalnız \"⏳ YAPILACAK\" olanlar bekleyen iştir."

// v48: SAYFA satırı yoksa gösterilen adres
const jpAnaSayfa = "https://example.pages.dev/"

// jpBilgi: önbellek tazeyse onu, değilse siteden çeker; site olmazsa eski önbellek (eski=true)
func jpBilgi(ctx context.Context) (metin string, eski bool, err error) {
	if st, e := os.Stat(jpOnbellekYol); e == nil && time.Since(st.ModTime()) < jpTaze {
		if b, e := os.ReadFile(jpOnbellekYol); e == nil && len(b) > 0 {
			return string(b), false, nil
		}
	}
	b, err := jpIndir(ctx)
	if err == nil {
		_ = dosyaYazAtomik(jpOnbellekYol, b, 0600)
		return string(b), false, nil
	}
	if o, e := os.ReadFile(jpOnbellekYol); e == nil && len(o) > 0 {
		return string(o), true, nil
	}
	return "", false, err
}

func jpIndir(ctx context.Context) ([]byte, error) {
	anahtar := oku(jpAnahtarYol, "")
	if anahtar == "" {
		return nil, fmt.Errorf("jp anahtarı yok")
	}
	ictx, iptal := context.WithTimeout(ctx, 20*time.Second)
	defer iptal()
	req, err := http.NewRequestWithContext(ictx, http.MethodGet, jpBilgiURL, nil)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Cookie", "jp26="+anahtar)
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("jp bilgi: bağlantı")
	}
	defer res.Body.Close()
	if res.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("jp bilgi: HTTP %d", res.StatusCode)
	}
	b, err := io.ReadAll(io.LimitReader(res.Body, jpBoyut+1))
	if err != nil || len(b) == 0 || len(b) > jpBoyut || !utf8.Valid(b) {
		return nil, fmt.Errorf("jp bilgi: geçersiz içerik")
	}
	return b, nil
}

// jpKes: en çok n karakter (rune), kesilirse "…"
func jpKes(s string, n int) string {
	if utf8.RuneCountInString(s) <= n {
		return s
	}
	r := []rune(s)
	return string(r[:n-1]) + "…"
}

func jpKomut(ctx context.Context, soru string) string {
	if soru == "" {
		return "🗾 gezi için sorunuzu komutla yazın efendim, örneğin: /jp 12 Kasım'da ne yapıyoruz?"
	}
	jctx, iptal := context.WithTimeout(ctx, butce(50*time.Second))
	defer iptal()
	belge, eski, err := jpBilgi(jctx)
	if err != nil {
		fmt.Fprintln(os.Stderr, "jp:", err)
		return "Gezi bilgisine şu an ulaşamıyorum efendim."
	}
	yy := sunucuYollar
	if yy.anahtar == "" {
		yy.anahtar, yy.model = "/data/asistan/gemini-anahtar", "/data/asistan/gemini-model"
	}
	anahtar := oku(yy.anahtar, "")
	if anahtar == "" {
		return "Şu an cevap veremiyorum efendim."
	}
	ist := istek{
		SystemInstruction: icerik{Parts: []parca{{Text: jpTalimat}}},
		Contents:          []icerik{{Role: "user", Parts: []parca{{Text: "BELGE:\n" + belge + "\n\nSORU: " + soru}}}},
		GenerationConfig:  map[string]any{"thinkingConfig": map[string]any{"thinkingLevel": "low"}},
	}
	modeller := sogumaSirala(strings.Split(oku(yy.model, varsayilanModel), ","), "llm:")
	y, model, kod := llmYaris(jctx, modeller, ist, anahtar, llmMetinYedek, 2)
	if kod != yarisTamam || len(y.Candidates) == 0 {
		return "Üzgünüm efendim, şu an bağlantı kuramıyorum."
	}
	var c strings.Builder
	for _, p := range y.Candidates[0].Content.Parts {
		c.WriteString(p.Text)
	}
	cevap := strings.TrimSpace(c.String())
	if cevap == "" {
		return "Üzgünüm efendim, cevabı anlayamadım."
	}
	fmt.Fprintf(os.Stderr, "jp: %s, %d karakter\n", model, utf8.RuneCountInString(cevap))
	if eski {
		cevap = "(Siteye ulaşılamadı; son kaydedilen bilgilerle) " + cevap
	}
	return jpKes(jpEtiket+"\n"+cevap, jpAzami)
}

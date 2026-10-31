// assembly.go (21.10.2026, Claude; kole-gemini v23, OTONOM Sıra 7 madde 41): Hey Köle sesli sorusunda Gemini sesli
// modellerinin hepsi düşerse (503 / kota / ağ) konuşma AssemblyAI ile metne çevrilir, soru metin modellerine gider.
// kullanıcı: "sana verdiğim diğer apileri de bağlayabilirsin sisteme". Anahtar /data/asistan/assembly-anahtar (Telegram
// /anahtar assembly <anahtar>; mesaj sohbetten silinir). Süre: asistan sınırı (butce) içinde, metne çevirme en çok 10 sn.
// Gizlilik: ses yalnız bu yedek yolda gönderilir; döküm alındıktan sonra AssemblyAI'dan silinir (DELETE /v2/transcript).
// Günlüğe sesin metni YAZILMAZ (yalnız uzunluk ve süre); anahtar hata metinlerinden çıkarılır.
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
	"time"
	"unicode/utf8"
)

var (
	assemblyAnahtarYol = "/data/asistan/assembly-anahtar"
	assemblyTaban      = "https://api.assemblyai.com"
	assemblyBekleme    = 400 * time.Millisecond
)

func assemblyIstek(req *http.Request, anahtar string, v any) error {
	req.Header.Set("authorization", anahtar)
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("%s", strings.ReplaceAll(err.Error(), anahtar, "***"))
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<20))
	if res.StatusCode/100 != 2 {
		var h struct {
			Error string `json:"error"`
		}
		_ = json.Unmarshal(b, &h)
		return fmt.Errorf("assembly HTTP %d %s", res.StatusCode, strings.ReplaceAll(h.Error, anahtar, "***"))
	}
	if v == nil {
		return nil
	}
	return json.Unmarshal(b, v)
}

// assemblyYaziya: WAV -> Türkçe metin (yükle, döküm iste, tamamlanana kadar sor, sonra AssemblyAI'dan sil).
func assemblyYaziya(ctx context.Context, anahtar string, wav []byte) (string, error) {
	req, _ := http.NewRequestWithContext(ctx, http.MethodPost, assemblyTaban+"/v2/upload", bytes.NewReader(wav))
	req.Header.Set("content-type", "application/octet-stream")
	var up struct {
		UploadURL string `json:"upload_url"`
	}
	if err := assemblyIstek(req, anahtar, &up); err != nil {
		return "", err
	}
	if up.UploadURL == "" {
		return "", fmt.Errorf("assembly: yükleme adresi yok")
	}
	b, _ := json.Marshal(map[string]any{"audio_url": up.UploadURL, "language_code": "tr", "punctuate": true,
		"format_text": true})
	req, _ = http.NewRequestWithContext(ctx, http.MethodPost, assemblyTaban+"/v2/transcript", bytes.NewReader(b))
	req.Header.Set("content-type", "application/json")
	var tr struct {
		ID     string `json:"id"`
		Status string `json:"status"`
		Text   string `json:"text"`
		Error  string `json:"error"`
	}
	if err := assemblyIstek(req, anahtar, &tr); err != nil {
		return "", err
	}
	if tr.ID == "" {
		return "", fmt.Errorf("assembly: döküm kimliği yok")
	}
	defer func() { // gizlilik: döküm + yüklenen ses AssemblyAI'dan silinir (en çok 3 sn)
		dctx, di := context.WithTimeout(context.Background(), 3*time.Second)
		defer di()
		dr, _ := http.NewRequestWithContext(dctx, http.MethodDelete, assemblyTaban+"/v2/transcript/"+tr.ID, nil)
		if err := assemblyIstek(dr, anahtar, nil); err != nil {
			fmt.Fprintln(os.Stderr, "assembly silme:", err)
		}
	}()
	for tr.Status != "completed" {
		if tr.Status == "error" {
			return "", fmt.Errorf("assembly döküm hatası: %s", tr.Error)
		}
		select {
		case <-ctx.Done():
			return "", ctx.Err()
		case <-time.After(assemblyBekleme):
		}
		req, _ = http.NewRequestWithContext(ctx, http.MethodGet, assemblyTaban+"/v2/transcript/"+tr.ID, nil)
		if err := assemblyIstek(req, anahtar, &tr); err != nil {
			return "", err
		}
	}
	return strings.TrimSpace(tr.Text), nil
}

// assemblyYedek: sesli istekteki son kullanıcı içeriğini (ses) AssemblyAI dökümüyle değiştirip metin modellerine sorar.
// Döner: yanıt, model ("… (assembly)"), anlaşılan metin, başarı.
func assemblyYedek(ctx context.Context, wav []byte, ist istek, modeller []string, anahtar string) (yanit, string, string, bool) {
	aa := oku(assemblyAnahtarYol, "")
	if aa == "" || len(wav) < 1000 || len(ist.Contents) == 0 {
		return yanit{}, "", "", false
	}
	if butce(time.Minute) < 8*time.Second {
		fmt.Fprintln(os.Stderr, "assembly yedek: süre yetmez")
		return yanit{}, "", "", false
	}
	t0 := time.Now()
	actx, ai := context.WithTimeout(ctx, butce(10*time.Second))
	metin, err := assemblyYaziya(actx, aa, wav)
	ai()
	if err != nil {
		fmt.Fprintln(os.Stderr, "assembly yedek:", err)
		return yanit{}, "", "", false
	}
	if metin == "" {
		fmt.Fprintln(os.Stderr, "assembly yedek: boş döküm")
		return yanit{}, "", "", false
	}
	fmt.Fprintf(os.Stderr, "assembly yedek: %d bayt ses -> %d harf, %d ms\n", len(wav), utf8.RuneCountInString(metin),
		time.Since(t0).Milliseconds())
	ist2 := ist
	ist2.Contents = append(append([]icerik{}, ist.Contents[:len(ist.Contents)-1]...), icerik{Role: "user",
		Parts: []parca{{Text: metin + "\n('soru' alanına bu soruyu, 'cevap' alanına cevabını yaz.)"}}})
	lctx, li := context.WithTimeout(ctx, butce(time.Minute))
	defer li()
	y, model, kod := llmYaris(lctx, sogumaSirala(modeller, "llm:"), ist2, anahtar, llmMetinYedek, 2)
	if kod != yarisTamam {
		fmt.Fprintln(os.Stderr, "assembly yedek: metin modelleri de cevap vermedi")
		return yanit{}, "", "", false
	}
	return y, model + " (assembly)", metin, true
}

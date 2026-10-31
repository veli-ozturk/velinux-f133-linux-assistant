// denetim_v42_test.go (23.10.2026, Claude; kole-gemini v42, scripts/598): denetim v2 düzeltmeleri (#5, #8, #10, #11,
// #17, #18, #20, R2, canlı uzatma sınırı). Sahte sunucularla; gerçek Telegram / Gemini / köprüye dokunmaz.
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

// #5: kilit doluyken --konus konuşan cevabın mikrofon bayrağını silmez
func TestDenetimKonusKilitDolu(t *testing.T) {
	d := t.TempDir()
	eskiM, eskiK := mikBirakYol, ttsKilitYol
	mikBirakYol, ttsKilitYol = filepath.Join(d, "mik"), filepath.Join(d, "kilit")
	t.Cleanup(func() { mikBirakYol, ttsKilitYol = eskiM, eskiK })
	anahtar := filepath.Join(d, "anahtar-yok")
	t.Setenv("KOLE_KONUS_METIN", "Merhaba efendim.")
	_ = os.WriteFile(ttsKilitYol, []byte("gemini 1\n"), 0644)
	_ = os.WriteFile(mikBirakYol, []byte("tts 1\n"), 0644)
	if rc := konusCalistir(anahtar); rc != 3 || !dosyaVar(mikBirakYol) {
		t.Fatalf("kilit doluyken konuşan cevabın bayrağı silindi: rc %d, bayrak %v", rc, dosyaVar(mikBirakYol))
	}
	_ = os.Remove(ttsKilitYol)
	_ = os.WriteFile(mikBirakYol, []byte("kopru 12\n"), 0644) // uzaktan dinleme: alınamaz, dokunulmaz
	if rc := konusCalistir(anahtar); rc != 4 || !dosyaVar(mikBirakYol) || dosyaVar(ttsKilitYol) {
		t.Fatalf("başkasının bayrağı silinmemeli, kilit bırakılmalı: rc %d", rc)
	}
	_ = os.Remove(mikBirakYol)
	if rc := konusCalistir(anahtar); rc != 4 || dosyaVar(mikBirakYol) || dosyaVar(ttsKilitYol) {
		t.Fatalf("kilidi alan konuşma bitince kilit ve bayrak temizlenmeli: rc %d", rc)
	}
}

// #8: bütün tts.log yazarları 64 KB'ta sıfırlar
func TestDenetimTtsGunlukSiniri(t *testing.T) {
	eski := ttsGunlukYol
	ttsGunlukYol = filepath.Join(t.TempDir(), "tts.log")
	t.Cleanup(func() { ttsGunlukYol = eski })
	_ = os.WriteFile(ttsGunlukYol, bytes.Repeat([]byte("x"), 70<<10), 0644)
	hazirGunluk("deneme %d", 1)
	b, _ := os.ReadFile(ttsGunlukYol)
	if len(b) > 1024 || !strings.Contains(string(b), "deneme 1") {
		t.Fatalf("64 KB üstü günlük sıfırlanmalı: %d bayt", len(b))
	}
}

// #10: iki yazar aynı anda: içerik karışmaz, hata yok, geçici dosya kalmaz, izin korunur
func TestDenetimAtomikYazEsZamanli(t *testing.T) {
	d := t.TempDir()
	yol := filepath.Join(d, "asistan-gecmis.json")
	var wg sync.WaitGroup
	for i := 0; i < 2; i++ {
		wg.Add(1)
		go func(c byte) {
			defer wg.Done()
			b := bytes.Repeat([]byte{c}, 32<<10)
			for k := 0; k < 100; k++ {
				if err := dosyaYazAtomik(yol, b, 0640); err != nil {
					t.Error(err)
					return
				}
			}
		}(byte('a' + i))
	}
	wg.Wait()
	b, err := os.ReadFile(yol)
	if err != nil || len(b) != 32<<10 || (bytes.Count(b, []byte{'a'}) != len(b) && bytes.Count(b, []byte{'b'}) != len(b)) {
		t.Fatalf("karışık ya da eksik içerik: %d bayt %v", len(b), err)
	}
	if st, _ := os.Stat(yol); st.Mode().Perm() != 0640 {
		t.Fatalf("izin %v", st.Mode().Perm())
	}
	if es, _ := os.ReadDir(d); len(es) != 1 {
		t.Fatalf("geçici dosya kaldı: %v", es)
	}
}

// soruOrtam: sahte Gemini (her isteğe metin; istekte varsa önce o çalışır) ve geçici yollar
func soruOrtam(t *testing.T, metin string, istekte func(y soruYollar)) soruYollar {
	d := t.TempDir()
	y := soruYollar{anahtar: filepath.Join(d, "anahtar"), model: filepath.Join(d, "model"), yuz: filepath.Join(d, "yuz"),
		gecmis: filepath.Join(d, "gecmis.json"), sehir: filepath.Join(d, "sehir"), eylem: filepath.Join(d, "eylem.json"),
		sure: 10 * time.Second}
	_ = os.WriteFile(y.anahtar, []byte("KEY\n"), 0600)
	_ = os.WriteFile(y.model, []byte("m1\n"), 0644)
	govde, _ := json.Marshal(map[string]any{"candidates": []any{map[string]any{"content": map[string]any{
		"parts": []any{map[string]any{"text": metin}}}}}})
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		_, _ = io.ReadAll(r.Body)
		if istekte != nil {
			istekte(y)
		}
		_, _ = w.Write(govde)
	}))
	eskiL, eskiS, eskiH, eskiA := llmURLTaban, sogumaYol, haberOnbellekYol, asistanGunluk
	eskiB, eskiC := surecBas, cikisSiniri
	llmURLTaban, sogumaYol = srv.URL+"/", filepath.Join(d, "s.json")
	haberOnbellekYol, asistanGunluk = filepath.Join(d, "h.json"), filepath.Join(d, "asistan.log")
	t.Cleanup(func() {
		srv.Close()
		llmURLTaban, sogumaYol, haberOnbellekYol, asistanGunluk = eskiL, eskiS, eskiH, eskiA
		surecBas, cikisSiniri = eskiB, eskiC
	})
	return y
}

// #10: soru sürerken başka süreç geçmişe tur yazdı: o tur kaybolmaz
func TestDenetimSoruGecmisEsZamanli(t *testing.T) {
	bir, iki := sonuc{Soru: "bir?", Cevap: "Bir."}, sonuc{Soru: "iki?", Cevap: "İki."}
	y := soruOrtam(t, `{"soru":"üç?","cevap":"Üç efendim."}`, func(y soruYollar) {
		gecmisYaz(y.gecmis, gecmis{Tur: []sonuc{bir, iki}}) // ayrı süreçteki Hey Köle sorusu bu arada bitti
	})
	gecmisYaz(y.gecmis, gecmis{Tur: []sonuc{bir}})
	s, rc := soruIsle(y, soruIstek{Metin: "üç?"})
	g := gecmisOku(y.gecmis)
	if rc != 0 || s.Cevap != "Üç efendim." || len(g.Tur) != 3 || g.Tur[1].Soru != "iki?" || g.Tur[2].Soru != "üç?" {
		t.Fatalf("rc %d, cevap %q, geçmiş %+v", rc, s.Cevap, g.Tur)
	}
}

// #18: bozuk/yarım JSON cevap seslendirilmez (rc 6, geçmiş yok); düz metin eskisi gibi
func TestDenetimSoruHamJSON(t *testing.T) {
	for _, c := range []struct {
		metin string
		rc    int
	}{
		{`{"soru":"saat kaç","cevap":"Saat on`, 6},
		{` {"soru":"saat kaç","cevap":""}`, 6},
		{"Saat on efendim.", 0},
	} {
		y := soruOrtam(t, c.metin, nil)
		s, rc := soruIsle(y, soruIstek{Metin: "saat kaç"})
		if rc != c.rc || (rc == 6 && dosyaVar(y.gecmis)) || (rc == 0 && s.Cevap != "Saat on efendim.") {
			t.Fatalf("%q: rc %d, cevap %q, geçmiş %v", c.metin, rc, s.Cevap, dosyaVar(y.gecmis))
		}
	}
}

// #11 + #17: ilk ses parçasından sonra susan akış akisSessizSure'de kesilir (gelen kadarı çalınır); akışa en çok 700
// karakter gider
func TestDenetimAkisSessizlikVeSinir(t *testing.T) {
	_, anahtar := akisOrtam(t)
	eskiListe, eskiSessiz := ttsGeminiModelleri, akisSessizSure
	ttsGeminiModelleri, akisSessizSure = nil, 300*time.Millisecond
	t.Cleanup(func() { ttsGeminiModelleri, akisSessizSure = eskiListe, eskiSessiz })
	k := yeniSahteKopru(t)
	defer k.l.Close()
	kopruAdres = k.l.Addr().String()
	var mu sync.Mutex
	var akisGovde []string
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		if !strings.Contains(string(b), `"stream":true`) { // karşılama sesi (tam yol)
			_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` +
				pcmB64(sinus(24000, 440, 2400, 3000)) + `"}]}]}`))
			return
		}
		mu.Lock()
		akisGovde = append(akisGovde, string(b))
		mu.Unlock()
		w.Header().Set("Content-Type", "text/event-stream")
		fmt.Fprintf(w, "data: {\"event_type\":\"step.delta\",\"delta\":{\"type\":\"audio\",\"mime_type\":\"audio/l16;rate=24000\",\"data\":\"%s\"}}\n\n",
			pcmB64(sinus(24000, 440, 4800, 3000)))
		w.(http.Flusher).Flush()
		select { // takıldı: bağlantı açık, veri yok
		case <-r.Context().Done():
		case <-time.After(10 * time.Second):
		}
	}))
	defer srv.Close()
	ttsURL = srv.URL
	t0 := time.Now()
	rc := akisCalistirMetin(anahtar, strings.Repeat("a", 700)+"ЖЖЖ")
	sure := time.Since(t0)
	g, _ := os.ReadFile(ttsGunlukYol)
	if rc != 0 || sure > 4*time.Second || !strings.Contains(string(g), "yarıda kesildi") {
		t.Fatalf("takılan akış bekçide kesilmeli: rc %d, %v\n%s", rc, sure, g)
	}
	k.mu.Lock()
	n := len(k.akis)
	k.mu.Unlock()
	if n <= 16000 {
		t.Fatalf("gelen parça çalınmalı: köprüye %d örnek", n)
	}
	mu.Lock()
	defer mu.Unlock()
	if len(akisGovde) != 1 || strings.Contains(akisGovde[0], "Ж") || !strings.Contains(akisGovde[0], strings.Repeat("a", 700)) {
		t.Fatalf("akışa en çok 700 karakter gitmeli (%d istek)", len(akisGovde))
	}
}

// #11: tam yolda takılan sağlayıcı ttsDenemeSure'de bırakılır, sıradaki denenir
func TestDenetimTtsDenemeSuresi(t *testing.T) {
	if ttsDenemeSure+700*ttsDenemeKarakter < 700*akisHarfMs*time.Millisecond { // 613: ölçülen üretim hızı sığmalı
		t.Fatalf("700 karakterlik sağlayıcı süresi Gemini ölçümünden kısa")
	}
	y := []*string{&sogumaYol, &ttsKotaYol, &ttsGunlukYol, &ttsSesYol, &ttsStilYol, &ttsModellerYol, &ttsModelYol,
		&gcloudAnahtarYol, &azureAnahtarYol, &elevenAnahtarYol}
	eskiY := make([]string, len(y))
	for i, p := range y {
		eskiY[i] = *p
	}
	eskiK := ttsDenemeKarakter
	t.Cleanup(func() {
		for i, p := range y {
			*p = eskiY[i]
		}
		ttsDenemeKarakter = eskiK
	})
	ttsDenemeKarakter = 0
	d := t.TempDir()
	sogumaYol, ttsKotaYol, ttsGunlukYol = filepath.Join(d, "s.json"), filepath.Join(d, "kota"), filepath.Join(d, "tts.log")
	ttsSesYol, ttsStilYol, ttsModellerYol = filepath.Join(d, "ses"), filepath.Join(d, "stil"), filepath.Join(d, "yok")
	ttsModelYol = filepath.Join(d, "model")
	gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "g"), filepath.Join(d, "a"), filepath.Join(d, "e")
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	eskiListe, eskiSure, eskiURL, eskiG := ttsGeminiModelleri, ttsDenemeSure, ttsURL, gcloudURL
	t.Cleanup(func() { ttsGeminiModelleri, ttsDenemeSure, ttsURL, gcloudURL = eskiListe, eskiSure, eskiURL, eskiG })
	ttsGeminiModelleri, ttsDenemeSure = []string{"m2-tts"}, 300*time.Millisecond
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		switch {
		case r.URL.Path != "/gemini":
			w.WriteHeader(403)
		case strings.Contains(string(b), `"m1-tts"`): // takıldı
			select {
			case <-r.Context().Done():
			case <-time.After(5 * time.Second):
			}
		default:
			_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` +
				pcmB64(sinus(24000, 440, 2400, 3000)) + `"}]}]}`))
		}
	}))
	defer srv.Close()
	ttsURL, gcloudURL = srv.URL+"/gemini", srv.URL+"/gcloud"
	t0 := time.Now()
	x, et, err := ttsUretSec(context.Background(), "GEMKEY1234567890", "Merhaba.", false)
	if err != nil || len(x) != 1600 || !strings.Contains(et, "m2-tts") || time.Since(t0) > 2*time.Second {
		t.Fatalf("takılan m1 bırakılıp m2 denenmeli: %v %d %q %v", err, len(x), et, time.Since(t0))
	}
}

// #20: eksik fotoğraf yalnız metinle gider; Telegram'ın reddettiği dosya .bozuk olur, sıradakiler gider; 5xx ve
// jeton/sohbet hatası (403) dosyayı bekletir
func TestDenetimBildirimKotuDosya(t *testing.T) {
	d := t.TempDir()
	eskiJ, eskiS, eskiK, eskiT := telegramJeton, telegramSohbet, kuyrukDizin, tgTaban
	t.Cleanup(func() { telegramJeton, telegramSohbet, kuyrukDizin, tgTaban = eskiJ, eskiS, eskiK, eskiT })
	telegramJeton, telegramSohbet, kuyrukDizin = filepath.Join(d, "jeton"), filepath.Join(d, "sohbet"), filepath.Join(d, "kuyruk")
	_ = os.WriteFile(telegramJeton, []byte("J\n"), 0600)
	_ = os.WriteFile(telegramSohbet, []byte("42\n"), 0600)
	_ = os.MkdirAll(kuyrukDizin, 0755)
	var mu sync.Mutex
	var giden []string
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		_ = r.ParseForm()
		m := r.PostForm.Get("text")
		mu.Lock()
		giden = append(giden, m)
		mu.Unlock()
		switch m {
		case "kötü":
			w.WriteHeader(400)
			_, _ = io.WriteString(w, `{"ok":false,"error_code":400,"description":"Bad Request: message is too long"}`)
		case "sunucu":
			w.WriteHeader(502)
		case "engel":
			w.WriteHeader(403)
			_, _ = io.WriteString(w, `{"ok":false,"error_code":403,"description":"Forbidden: bot was blocked by the user"}`)
		default:
			_, _ = io.WriteString(w, `{"ok":true,"result":{}}`)
		}
	}))
	defer srv.Close()
	tgTaban = srv.URL
	kuyruk := func(ad, s string) { _ = os.WriteFile(filepath.Join(kuyrukDizin, ad), []byte(s), 0644) }
	kuyruk("01.txt", "foto:"+filepath.Join(d, "silinmis.jpg")+"\nhareket var\n")
	kuyruk("02.txt", "kötü\n")
	kuyruk("03.txt", "üç\n")
	if err := bildirimGonder(); err != nil {
		t.Fatal(err)
	}
	mu.Lock()
	g := strings.Join(giden, "|")
	mu.Unlock()
	kalan, _ := filepath.Glob(filepath.Join(kuyrukDizin, "*"))
	if g != "hareket var|kötü|üç" || len(kalan) != 1 || filepath.Base(kalan[0]) != "02.txt.bozuk" {
		t.Fatalf("giden %q, kalan %v", g, kalan)
	}
	for _, m := range []string{"sunucu", "engel"} {
		kuyruk("04.txt", m+"\n")
		if err := bildirimGonder(); err == nil || !dosyaVar(filepath.Join(kuyrukDizin, "04.txt")) {
			t.Fatalf("%s: hata dönmeli ve dosya beklemeli: %v", m, err)
		}
	}
}

// R2: sahip yalnız yerel izinle ve özel sohbetten öğrenilir; izin yazma başarılıysa silinir
func TestDenetimSahipOgrenme(t *testing.T) {
	d := t.TempDir()
	eskiS, eskiI, eskiT := telegramSohbet, telegramSahipOgren, tgTaban
	t.Cleanup(func() { telegramSohbet, telegramSahipOgren, tgTaban = eskiS, eskiI, eskiT })
	telegramSohbet, telegramSahipOgren = filepath.Join(d, "sohbet"), filepath.Join(d, "izin")
	var mu sync.Mutex
	istek, guncelleme := 0, `[{"message":{"chat":{"id":55}}},{"message":{"chat":{"id":-77}}}]`
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		mu.Lock()
		istek++
		g := guncelleme
		mu.Unlock()
		_, _ = io.WriteString(w, `{"ok":true,"result":`+g+`}`)
	}))
	defer srv.Close()
	tgTaban = srv.URL
	sayi := func() int {
		mu.Lock()
		defer mu.Unlock()
		return istek
	}
	ctx := context.Background()
	if _, err := telegramSohbetBul(ctx, "J"); err == nil || !strings.Contains(err.Error(), "sohbet yok") || sayi() != 0 || dosyaVar(telegramSohbet) {
		t.Fatalf("izin yokken öğrenilmemeli (getUpdates de yok): %v, istek %d", err, sayi())
	}
	_ = os.WriteFile(telegramSahipOgren, nil, 0600)
	if s, err := telegramSohbetBul(ctx, "J"); err != nil || s != "55" || oku(telegramSohbet, "") != "55" || dosyaVar(telegramSahipOgren) {
		t.Fatalf("son mesaj grup: özel sohbet 55 sahip olmalı, izin silinmeli: %q %v", s, err)
	}
	if s, err := telegramSohbetBul(ctx, "J"); err != nil || s != "55" || sayi() != 1 {
		t.Fatalf("kayıtlı sahip dosyadan: %q %v, istek %d", s, err, sayi())
	}
	_ = os.Remove(telegramSohbet)
	_ = os.WriteFile(telegramSahipOgren, nil, 0600)
	mu.Lock()
	guncelleme = `[{"message":{"chat":{"id":-77}}}]`
	mu.Unlock()
	if _, err := telegramSohbetBul(ctx, "J"); err == nil || dosyaVar(telegramSohbet) || !dosyaVar(telegramSahipOgren) {
		t.Fatalf("yalnız grup mesajı: öğrenilmemeli, izin durmalı: %v", err)
	}
	mu.Lock()
	guncelleme = `[{"message":{"chat":{"id":55}}}]`
	mu.Unlock()
	telegramSohbet = filepath.Join(d, "yok", "sohbet")
	if _, err := telegramSohbetBul(ctx, "J"); err == nil || !dosyaVar(telegramSahipOgren) {
		t.Fatalf("sohbet yazılamazsa izin silinmemeli: %v", err)
	}
}

// canlı: uzat, sürerken /canli ve başka süreçten uzat/yeniden bitişi oturum başı + canliAzami'yi aşamaz
func TestDenetimCanliUzatmaSiniri(t *testing.T) {
	canliKur(t)
	canliSure = time.Minute
	simdi := time.Now()
	lim := simdi.Add(10 * time.Second)
	canliDurum.Lock()
	canliDurum.aktif, canliDurum.bas, canliDurum.bitis = true, lim.Add(-canliAzami), simdi.Add(5*time.Second)
	canliDurum.Unlock()
	t.Cleanup(func() {
		canliDurum.Lock()
		canliDurum.aktif = false
		canliDurum.Unlock()
	})
	kontrol := func(ad string) {
		t.Helper()
		if b := canliBitis(); !b.Equal(lim) {
			t.Fatalf("%s: bitiş +%v, sınır +%v (oturum başı + canliAzami)", ad, b.Sub(simdi), lim.Sub(simdi))
		}
	}
	ctx := context.Background()
	tgCanli(ctx, "J", "42", "uzat")
	kontrol("uzat")
	tgCanli(ctx, "J", "42", "")
	kontrol("sürerken /canli")
	for _, k := range []string{"uzat", "yeniden"} {
		if err := canliKontrolYaz(k); err != nil {
			t.Fatal(err)
		}
		if canliKontrolUygula() {
			t.Fatal(k + ": durdurmamalı")
		}
		kontrol("başka süreçten " + k)
	}
}

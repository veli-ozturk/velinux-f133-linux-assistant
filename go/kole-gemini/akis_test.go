package main

import (
	"bufio"
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"io"
	"math/rand"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"
)

func TestAkisOrnekleyici(t *testing.T) {
	r := rand.New(rand.NewSource(7))
	x := make([]int16, 31200)
	for i := range x {
		x[i] = int16(r.Intn(20000) - 10000)
	}
	toplu := yenidenOrnekle(x, 24000, 16000)
	for deneme := 0; deneme < 5; deneme++ {
		a := yeniAkisOrnekleyici(24000, 16000)
		var y []int16
		for i := 0; i < len(x); {
			n := 1 + r.Intn(3000)
			if i+n > len(x) {
				n = len(x) - i
			}
			y = append(y, a.ekle(x[i:i+n], false)...)
			i += n
		}
		y = append(y, a.ekle(nil, true)...)
		if len(y) != len(toplu) {
			t.Fatalf("uzunluk %d, toplu %d", len(y), len(toplu))
		}
		for i := range y {
			if y[i] != toplu[i] {
				t.Fatalf("ornek %d: %d != %d", i, y[i], toplu[i])
			}
		}
	}
	a := yeniAkisOrnekleyici(16000, 16000)
	if y := append(a.ekle(x[:10], false), a.ekle(x[10:20], true)...); len(y) != 20 || y[15] != x[15] {
		t.Fatal("ayni hiz")
	}
}

func sesB64(n int, v int16) string {
	b := make([]byte, 2*n)
	for i := 0; i < n; i++ {
		binary.LittleEndian.PutUint16(b[2*i:], uint16(v))
	}
	return base64.StdEncoding.EncodeToString(b)
}

func TestAkisAyristirici(t *testing.T) {
	delta := func(n int, v int16) string {
		return `{"event_type":"step.delta","delta":{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` + sesB64(n, v) + `"}}`
	}
	tam := `{"event_type":"interaction.complete","interaction":{"steps":[{"content":[{"type":"audio","data":"` + sesB64(999, 9) + `"}]}]}}`
	topla := func(g string) (int, []int, error) {
		var boy []int
		n, err := akisSesParcalari(strings.NewReader(g), func(x []int16, hz int) error {
			if hz != 24000 {
				return fmt.Errorf("hz %d", hz)
			}
			boy = append(boy, len(x))
			return nil
		})
		return n, boy, err
	}
	sse := "event: step.delta\ndata: " + delta(100, 1) + "\n\n: yorum\ndata: " + delta(200, 2) + "\n\ndata: " + tam + "\n\n"
	if n, b, err := topla(sse); err != nil || n != 2 || fmt.Sprint(b) != "[100 200]" {
		t.Fatalf("SSE: %d %v %v", n, b, err)
	}
	ndjson := delta(50, 1) + "\n" + delta(60, 1) + "\n" + tam + "\n"
	if n, b, err := topla(ndjson); err != nil || n != 2 || fmt.Sprint(b) != "[50 60]" {
		t.Fatalf("NDJSON: %d %v %v", n, b, err)
	}
	dizi := "[{\n  \"event_type\": \"step.delta\",\n  \"delta\": {\"type\": \"audio\", \"data\": \"" + sesB64(30, 3) + "\"}\n}\n,\n" +
		"{\"event_type\":\"step.delta\",\"delta\":{\"type\":\"audio\",\"data\":\"" + sesB64(40, 3) + "\"}}\n]"
	if n, b, err := topla(dizi); err != nil || n != 2 || fmt.Sprint(b) != "[30 40]" {
		t.Fatalf("dizi: %d %v %v", n, b, err)
	}
	if n, b, err := topla("data: " + tam + "\n\n"); err != nil || n != 1 || fmt.Sprint(b) != "[999]" {
		t.Fatalf("yalniz tam ses: %d %v %v", n, b, err)
	}
	if _, _, err := topla("data: " + delta(10, 1) + "\n\ndata: {\"error\":{\"message\":\"kota\"}}\n\n"); err == nil || !strings.Contains(err.Error(), "kota") {
		t.Fatalf("hata olayi: %v", err)
	}
	if n, _, err := topla("data: {\"event_type\":\"step.delta\",\"delta\":{\"type\":\"text\",\"text\":\"x\"}}\n\n"); err != nil || n != 0 {
		t.Fatalf("metin deltasi ses sayilmamali: %d %v", n, err)
	}
}

// sahte köprü: PLAY_AKIS (kabul/ret) ve PLAY
type sahteKopru struct {
	mu       sync.Mutex
	l        net.Listener
	reddet   bool
	komutlar []string
	akis     []int16 // PLAY_AKIS ile gelen örnekler
	play     int     // PLAY bayt
	ilkSure  bool    // ilk gerçek (sıfır olmayan) örnek geldiğinde tts-sure var mıydı
}

func yeniSahteKopru(t *testing.T) *sahteKopru {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	k := &sahteKopru{l: l}
	go func() {
		for {
			c, err := l.Accept()
			if err != nil {
				return
			}
			k.isle(c)
		}
	}()
	return k
}

func (k *sahteKopru) isle(c net.Conn) {
	defer c.Close()
	br := bufio.NewReader(c)
	bas, _ := br.ReadString('\n')
	f := strings.Fields(bas)
	k.mu.Lock()
	k.komutlar = append(k.komutlar, strings.Join(f[1:], " "))
	k.mu.Unlock()
	if len(f) < 2 || f[0] != "KOPRU123" {
		c.Write([]byte("HATA anahtar\n"))
		return
	}
	switch f[1] {
	case "PLAY_AKIS":
		if k.reddet {
			c.Write([]byte("HATA hoparlor mesgul\n"))
			return
		}
		c.Write([]byte("PLAY_AKIS HAZIR 16000\n"))
		n := 0
		for {
			h := make([]byte, 2)
			if _, err := io.ReadFull(br, h); err != nil {
				return
			}
			m := int(binary.LittleEndian.Uint16(h))
			if m == 0 {
				break
			}
			b := make([]byte, m)
			if _, err := io.ReadFull(br, b); err != nil {
				return
			}
			k.mu.Lock()
			for i := 0; i < m/2; i++ {
				v := int16(binary.LittleEndian.Uint16(b[2*i:]))
				if v != 0 && !k.ilkSure {
					k.ilkSure = dosyaVar(ttsSureYol)
				}
				k.akis = append(k.akis, v)
			}
			k.mu.Unlock()
			n += m / 2
		}
		fmt.Fprintf(c, "OK %d 0\n", n)
	case "PLAY":
		boy, _ := strconv.Atoi(f[2])
		b := make([]byte, boy)
		_, _ = io.ReadFull(br, b)
		k.mu.Lock()
		k.play = boy
		k.mu.Unlock()
		c.Write([]byte("OK 0\n"))
	}
}

func akisOrtam(t *testing.T) (string, string) {
	d := t.TempDir()
	sogumaYol, ttsKotaYol, ttsGunlukYol = filepath.Join(d, "s.json"), filepath.Join(d, "kota"), filepath.Join(d, "tts.log")
	ttsSesYol, ttsStilYol, ttsModellerYol = filepath.Join(d, "ses"), filepath.Join(d, "stil"), filepath.Join(d, "yok")
	ttsModelYol, ttsKilitYol, ttsSureYol = filepath.Join(d, "model"), filepath.Join(d, "kilit"), filepath.Join(d, "sure")
	mikBirakYol, akisHataYol, ttsKapaliYol = filepath.Join(d, "mik"), filepath.Join(d, "hata"), filepath.Join(d, "kapali")
	selamPCMYol, selamMetinYol, selamSesYol = filepath.Join(d, "selam.pcm"), filepath.Join(d, "selam.txt"), filepath.Join(d, "selam.ses")
	kopruAnahtarYol = filepath.Join(d, "kopru-anahtar")
	_ = os.WriteFile(kopruAnahtarYol, []byte("KOPRU123\n"), 0600)
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	anahtar := filepath.Join(d, "anahtar")
	_ = os.WriteFile(anahtar, []byte("GEMKEY1234567890\n"), 0600)
	_ = os.WriteFile(ttsKilitYol, []byte("gemini 1\n"), 0644) // ana süreç almış gibi
	_ = os.WriteFile(mikBirakYol, []byte("tts 1\n"), 0644)
	return d, anahtar
}

// sahte TTS: akış isteğine (stream:true) 3 parça SSE; tam isteğe tek ses. kod: akışa dönülecek HTTP kodu (0: 200)
func sahteTTS(t *testing.T, akisKod int, sureVar *bool) *httptest.Server {
	return httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		if strings.Contains(string(b), `"stream":true`) {
			if akisKod != 0 {
				w.WriteHeader(akisKod)
				_, _ = w.Write([]byte(`{"error":{"message":"Rate limit exceeded"}}`))
				return
			}
			w.Header().Set("Content-Type", "text/event-stream")
			fl := w.(http.Flusher)
			for i := 0; i < 3; i++ {
				if i == 2 && sureVar != nil {
					*sureVar = dosyaVar(ttsSureYol) // son parçadan önce altyazı başlamış olmalı
				}
				fmt.Fprintf(w, "data: {\"event_type\":\"step.delta\",\"delta\":{\"type\":\"audio\",\"mime_type\":\"audio/l16;rate=24000\",\"data\":\"%s\"}}\n\n",
					pcmB64(sinus(24000, 440, 4800, 3000)))
				fl.Flush()
				time.Sleep(80 * time.Millisecond)
			}
			return
		}
		_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` +
			pcmB64(sinus(24000, 440, 2400, 3000)) + `"}]}]}`))
	}))
}

func TestAkisUctanUca(t *testing.T) {
	_, anahtar := akisOrtam(t)
	eskiListe := ttsGeminiModelleri
	defer func() { ttsGeminiModelleri = eskiListe }()
	ttsGeminiModelleri = nil
	k := yeniSahteKopru(t)
	defer k.l.Close()
	kopruAdres = k.l.Addr().String()
	var sureVar bool
	srv := sahteTTS(t, 0, &sureVar)
	defer srv.Close()
	ttsURL = srv.URL
	t.Setenv("KOLE_KONUS_METIN", "Merhaba efendim, bugün hava güzel.")
	if rc := akisCalistir(anahtar); rc != 0 {
		g, _ := os.ReadFile(ttsGunlukYol)
		t.Fatalf("rc %d\n%s", rc, g)
	}
	k.mu.Lock()
	defer k.mu.Unlock()
	var parca []int16
	for i := 0; i < 3; i++ {
		parca = append(parca, sinus(24000, 440, 4800, 3000)...)
	}
	beklenen := 16000 + len(yenidenOrnekle(parca, 24000, 16000))
	if len(k.akis) != beklenen || k.komutlar[0] != "PLAY_AKIS 60" {
		t.Fatalf("akis %d ornek (beklenen %d), komutlar %v", len(k.akis), beklenen, k.komutlar)
	}
	for i := 0; i < 16000; i++ {
		if k.akis[i] != 0 {
			t.Fatalf("on sessizlik %d. ornekte bozuk", i)
		}
	}
	if !sureVar || !k.ilkSure {
		t.Fatalf("tts-sure ilk seste yazilmali (sunucu %v, kopru %v)", sureVar, k.ilkSure)
	}
	if dosyaVar(ttsKilitYol) || dosyaVar(ttsSureYol) || dosyaVar(mikBirakYol) {
		t.Fatal("kilit/sure/mikrofon bayragi temizlenmeli")
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	if !strings.Contains(string(g), "tts akış: m1-tts, ilk ses") || strings.Contains(string(g), "GEMKEY") {
		t.Fatalf("gunluk:\n%s", g)
	}
	if !dosyaVar(selamPCMYol) {
		t.Fatal("selam uretilmeli (v14 davranisi)")
	}
}

func TestAkisYedekTamYol(t *testing.T) {
	_, anahtar := akisOrtam(t)
	eskiListe := ttsGeminiModelleri
	defer func() { ttsGeminiModelleri = eskiListe }()
	ttsGeminiModelleri = []string{"m2-tts"}
	k := yeniSahteKopru(t)
	defer k.l.Close()
	kopruAdres = k.l.Addr().String()
	srv := sahteTTS(t, 429, nil)
	defer srv.Close()
	ttsURL = srv.URL
	t.Setenv("KOLE_KONUS_METIN", "Yedek yol.")
	if rc := akisCalistir(anahtar); rc != 0 {
		g, _ := os.ReadFile(ttsGunlukYol)
		t.Fatalf("rc %d\n%s", rc, g)
	}
	k.mu.Lock()
	defer k.mu.Unlock()
	if k.play != 2*(16000+1600+3200) || len(k.akis) != 0 {
		t.Fatalf("PLAY %d bayt, akis %d, komutlar %v", k.play, len(k.akis), k.komutlar)
	}
	if _, ok := sogumaDurum("tts:gemini:m2-tts"); !ok { // m1 yedek tam yolda başarılı oldu: soğuması silindi
		t.Fatal("m2 akis 429 sogumasi yok")
	}
	if _, ok := sogumaDurum("tts:gemini:m1-tts"); ok {
		t.Fatal("m1 tam yolda basarili: soguma silinmeli")
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	if !strings.Contains(string(g), "tam yola geçiliyor") || !strings.Contains(string(g), "yedek tam yol") {
		t.Fatalf("gunluk:\n%s", g)
	}
}

func TestAkisKopruReddi(t *testing.T) {
	_, anahtar := akisOrtam(t)
	eskiListe := ttsGeminiModelleri
	defer func() { ttsGeminiModelleri = eskiListe }()
	ttsGeminiModelleri = nil
	k := yeniSahteKopru(t)
	k.reddet = true
	defer k.l.Close()
	kopruAdres = k.l.Addr().String()
	srv := sahteTTS(t, 0, nil)
	defer srv.Close()
	ttsURL = srv.URL
	t.Setenv("KOLE_KONUS_METIN", "Köprü meşgul.")
	if rc := akisCalistir(anahtar); rc != 0 {
		g, _ := os.ReadFile(ttsGunlukYol)
		t.Fatalf("rc %d\n%s", rc, g)
	}
	k.mu.Lock()
	defer k.mu.Unlock()
	var parca []int16
	for i := 0; i < 3; i++ {
		parca = append(parca, sinus(24000, 440, 4800, 3000)...)
	}
	if k.play != 2*(16000+len(yenidenOrnekle(parca, 24000, 16000))+3200) {
		t.Fatalf("biriktirilen PLAY %d bayt, komutlar %v", k.play, k.komutlar)
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	if !strings.Contains(string(g), "biriktir") {
		t.Fatalf("gunluk:\n%s", g)
	}
}

func TestAkisHazirBekle(t *testing.T) {
	d := t.TempDir()
	ttsSureYol, akisHataYol = filepath.Join(d, "sure"), filepath.Join(d, "hata")
	go func() {
		time.Sleep(150 * time.Millisecond)
		_ = os.WriteFile(ttsSureYol, []byte("on=1000 sure=2000\n"), 0644)
	}()
	if !akisHazirBekle(2 * time.Second) {
		t.Fatal("tts-sure gorunce true")
	}
	_ = os.Remove(ttsSureYol)
	_ = os.WriteFile(akisHataYol, []byte("1\n"), 0644)
	t0 := time.Now()
	if akisHazirBekle(2*time.Second) || time.Since(t0) > time.Second {
		t.Fatal("hata dosyasinda hemen false")
	}
	_ = os.Remove(akisHataYol)
	if akisHazirBekle(200 * time.Millisecond) {
		t.Fatal("zaman asiminda false")
	}
}

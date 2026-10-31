// canli_tg_test.go (22.10.2026, Claude; kole-gemini v30): Telegram içinde canlı izleme, sahte Telegram sunucusu ve sahte
// kamera karesiyle (gerçek kameraya/Telegram'a dokunmaz).
package main

import (
	"context"
	"io"
	"mime"
	"mime/multipart"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

type canliSahte struct {
	mu    sync.Mutex
	cagri []string
	alan  []map[string]string
}

func (c *canliSahte) say(m string) int {
	c.mu.Lock()
	defer c.mu.Unlock()
	n := 0
	for _, x := range c.cagri {
		if x == m {
			n++
		}
	}
	return n
}

func (c *canliSahte) son(m string) map[string]string {
	c.mu.Lock()
	defer c.mu.Unlock()
	for i := len(c.cagri) - 1; i >= 0; i-- {
		if c.cagri[i] == m {
			return c.alan[i]
		}
	}
	return nil
}

func canliKur(t *testing.T) (*canliSahte, string) {
	c := &canliSahte{}
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		p := strings.Split(r.URL.Path, "/")
		m := p[len(p)-1]
		a := map[string]string{}
		if mt, prm, _ := mime.ParseMediaType(r.Header.Get("Content-Type")); mt == "multipart/form-data" {
			mr := multipart.NewReader(r.Body, prm["boundary"])
			for {
				part, err := mr.NextPart()
				if err != nil {
					break
				}
				b, _ := io.ReadAll(part)
				if part.FileName() != "" {
					a["@"+part.FormName()] = string(b[:2])
				} else {
					a[part.FormName()] = string(b)
				}
			}
		} else {
			_ = r.ParseForm()
			for k := range r.PostForm {
				a[k] = r.PostForm.Get(k)
			}
		}
		if m == "editMessageMedia" {
			time.Sleep(canliTestGecikme) // v36: yükleme süresi taklidi
		}
		c.mu.Lock()
		c.cagri = append(c.cagri, m)
		c.alan = append(c.alan, a)
		c.mu.Unlock()
		if m == "sendPhoto" {
			_, _ = io.WriteString(w, `{"ok":true,"result":{"message_id":77}}`)
			return
		}
		_, _ = io.WriteString(w, `{"ok":true,"result":true}`)
	}))
	d := t.TempDir()
	eski := []any{tgTaban, canliBayrak, canliKare, canliSure, canliAralik, canliIlkBek, canliKalkan}
	tgTaban = srv.URL
	canliBayrak, canliKare = filepath.Join(d, "kamera-aktif"), filepath.Join(d, "kamera-son.jpg")
	canliSure, canliAralik, canliIlkBek = 3*time.Second, 200*time.Millisecond, 2*time.Second
	canliKalkan = filepath.Join(d, "kalkan")
	eskiPid, eskiKontrol := canliPid, canliKontrol // v41
	canliPid, canliKontrol = filepath.Join(d, "tg-canli.pid"), filepath.Join(d, "tg-canli-kontrol")
	t.Cleanup(func() { canliPid, canliKontrol = eskiPid, eskiKontrol })
	t.Cleanup(func() {
		srv.Close()
		tgTaban = eski[0].(string)
		canliBayrak, canliKare = eski[1].(string), eski[2].(string)
		canliSure, canliAralik, canliIlkBek = eski[3].(time.Duration), eski[4].(time.Duration), eski[5].(time.Duration)
		canliKalkan = eski[6].(string)
	})
	return c, d
}

// kamera-kayit taklidi: bayrak varken 100 ms'de bir yeni kare. Döndürdüğü işlev durdurur ve çıkışını bekler (yol
// değişkenleri başta kopyalanır: test temizliğiyle yarış olmasın).
func sahteKamera(t *testing.T) func() {
	bayrak, kare := canliBayrak, canliKare
	dur, bitti := make(chan struct{}), make(chan struct{})
	go func() {
		defer close(bitti)
		for i := 0; ; i++ {
			select {
			case <-dur:
				return
			case <-time.After(100 * time.Millisecond):
			}
			if dosyaVar(bayrak) {
				_ = os.WriteFile(kare, []byte{0xFF, 0xD8, byte(i), 0xFF, 0xD9}, 0644)
			}
		}
	}()
	return func() { close(dur); <-bitti }
}

func canliBekle(t *testing.T, kosul func() bool, sure time.Duration) bool {
	t0 := time.Now()
	for time.Since(t0) < sure {
		if kosul() {
			return true
		}
		time.Sleep(20 * time.Millisecond)
	}
	return kosul()
}

func TestCanliTelegram(t *testing.T) {
	c, _ := canliKur(t)
	defer sahteKamera(t)()
	ctx := context.Background()
	tgCanli(ctx, "J", "42", "")
	if !canliBekle(t, func() bool { return c.say("sendPhoto") == 1 }, 3*time.Second) {
		t.Fatalf("sendPhoto yok: %v", c.cagri)
	}
	b, _ := os.ReadFile(canliBayrak)
	if !strings.HasPrefix(string(b), "telegram ") {
		t.Fatalf("bayrak telegram değil: %q", b)
	}
	sp := c.son("sendPhoto")
	if sp["chat_id"] != "42" || sp["@photo"] != "\xff\xd8" || !strings.Contains(sp["reply_markup"], "/canli dur") ||
		!strings.Contains(sp["caption"], "Canlı") {
		t.Fatalf("sendPhoto alanları: %v", sp)
	}
	if !canliBekle(t, func() bool { return c.say("editMessageMedia") >= 3 }, 3*time.Second) {
		t.Fatalf("editMessageMedia az: %d", c.say("editMessageMedia"))
	}
	em := c.son("editMessageMedia")
	if em["message_id"] != "77" || em["@kare"] != "\xff\xd8" || !strings.Contains(em["media"], "attach://kare") {
		t.Fatalf("editMessageMedia alanları: %v", em)
	}
	tgCanli(ctx, "J", "42", "uzat")
	if time.Until(canliBitis()) < 4*time.Second {
		t.Fatalf("uzat çalışmadı: %v", time.Until(canliBitis()))
	}
	tgCanli(ctx, "J", "42", "dur")
	if !canliBekle(t, func() bool { return !canliSurdu() }, 2*time.Second) {
		t.Fatal("durmadı")
	}
	if dosyaVar(canliBayrak) {
		t.Fatal("bayrak silinmedi")
	}
	ec := c.son("editMessageCaption")
	if ec == nil || !strings.Contains(ec["caption"], "durduruldu") || !strings.Contains(ec["reply_markup"], "Yeniden") {
		t.Fatalf("bitiş açıklaması: %v", ec)
	}
	n := c.say("editMessageMedia")
	time.Sleep(600 * time.Millisecond)
	if c.say("editMessageMedia") != n {
		t.Fatal("durduktan sonra düzenleme sürdü")
	}
}

func TestCanliSureDolar(t *testing.T) {
	c, _ := canliKur(t)
	defer sahteKamera(t)()
	canliSure = 1 * time.Second
	tgCanli(context.Background(), "J", "42", "")
	if !canliBekle(t, func() bool { return c.say("sendPhoto") == 1 }, 3*time.Second) {
		t.Fatal("başlamadı")
	}
	if !canliBekle(t, func() bool { return !canliSurdu() }, 4*time.Second) {
		t.Fatal("süre dolunca durmadı")
	}
	if ec := c.son("editMessageCaption"); ec == nil || !strings.Contains(ec["caption"], "süre doldu") {
		t.Fatalf("bitiş açıklaması: %v", ec)
	}
}

func TestCanliKalkanVeYabanciBayrak(t *testing.T) {
	c, d := canliKur(t)
	_ = os.WriteFile(canliKalkan, []byte("1"), 0644)
	tgCanli(context.Background(), "J", "42", "")
	time.Sleep(300 * time.Millisecond)
	if canliSurdu() || c.say("sendPhoto") != 0 || c.say("sendMessage") != 1 || dosyaVar(canliBayrak) {
		t.Fatalf("kalkan açıkken başladı: %v", c.cagri)
	}
	if !strings.Contains(c.son("sendMessage")["text"], "kalkan") {
		t.Fatalf("kalkan iletisi: %v", c.son("sendMessage"))
	}
	_ = os.Remove(canliKalkan)
	// Merkez izliyor: bayrak "merkez ..." -> dokunulmaz, silinmez
	_ = os.WriteFile(canliBayrak, []byte("merkez 192.0.2.1\n"), 0644)
	canliBayrakYaz(time.Now().Add(time.Minute))
	canliBayrakSil()
	if b, _ := os.ReadFile(canliBayrak); !strings.HasPrefix(string(b), "merkez") {
		t.Fatalf("Merkez bayrağı değişti: %q", b)
	}
	// süresi geçmiş telegram bayrağı temizlenir, süresi sürmekte olan kalır
	_ = os.WriteFile(canliBayrak, []byte("telegram 1\n"), 0644)
	canliEskiTemizle()
	if dosyaVar(canliBayrak) {
		t.Fatal("eski telegram bayrağı silinmedi")
	}
	_ = os.WriteFile(canliBayrak, []byte("telegram 9999999999\n"), 0644)
	canliEskiTemizle()
	if !dosyaVar(canliBayrak) {
		t.Fatal("süren telegram bayrağı silindi")
	}
	_ = d
}

func TestCanliKareYok(t *testing.T) {
	c, _ := canliKur(t)
	canliIlkBek = 500 * time.Millisecond
	tgCanli(context.Background(), "J", "42", "")
	if !canliBekle(t, func() bool { return !canliSurdu() }, 3*time.Second) {
		t.Fatal("kare yokken durmadı")
	}
	if c.say("sendPhoto") != 0 || !strings.Contains(c.son("sendMessage")["text"], "kare gelmedi") || dosyaVar(canliBayrak) {
		t.Fatalf("kare yok durumu: %v", c.cagri)
	}
}

func TestTgBekleSn(t *testing.T) {
	if n := tgBekleSn(errString("telegram editMessageMedia: HTTP 429 Too Many Requests: retry after 7")); n != 7 {
		t.Fatalf("retry after: %d", n)
	}
	if n := tgBekleSn(errString("başka hata")); n != 0 {
		t.Fatalf("retry yok: %d", n)
	}
}

type errString string

func (e errString) Error() string { return string(e) }

// telegram_menu_test.go (21.10.2026, Claude; kole-gemini v22): sahte Telegram sunucusuyla menü, satır içi düğmeler,
// sol menü kurulumu, profil fotoğrafı ve uzun sorgu döngüsü.
package main

import (
	"context"
	"encoding/json"
	"io"
	"mime"
	"mime/multipart"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"sync"
	"testing"
	"time"
	"unicode/utf8"
)

type sahteTg struct {
	mu       sync.Mutex
	cagri    []string
	form     map[string][]url.Values
	logo     []byte
	logoJSON string
	cevap    map[string]string
	guncel   []string
}

func yeniSahteTg(t *testing.T) *sahteTg {
	s := &sahteTg{form: map[string][]url.Values{}, cevap: map[string]string{}}
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		parca := strings.Split(r.URL.Path, "/")
		metod := parca[len(parca)-1]
		s.mu.Lock()
		defer s.mu.Unlock()
		s.cagri = append(s.cagri, metod)
		if metod == "setMyProfilePhoto" {
			_, p, _ := mime.ParseMediaType(r.Header.Get("Content-Type"))
			mr := multipart.NewReader(r.Body, p["boundary"])
			for {
				part, err := mr.NextPart()
				if err != nil {
					break
				}
				b, _ := io.ReadAll(part)
				if part.FormName() == "photo" {
					s.logoJSON = string(b)
				} else if part.FormName() == "logo" {
					s.logo = b
				}
			}
		} else {
			_ = r.ParseForm()
			s.form[metod] = append(s.form[metod], r.PostForm)
		}
		if c, ok := s.cevap[metod]; ok {
			_, _ = io.WriteString(w, c)
			return
		}
		if metod == "getUpdates" {
			sonuc := "[]"
			if len(s.guncel) > 0 {
				sonuc, s.guncel = s.guncel[0], s.guncel[1:]
			}
			_, _ = io.WriteString(w, `{"ok":true,"result":`+sonuc+`}`)
			return
		}
		_, _ = io.WriteString(w, `{"ok":true,"result":true}`)
	}))
	eski := tgTaban
	tgTaban = srv.URL
	d := t.TempDir()
	eskiYol := []string{tgMenuBayrak, tgMenuDeneme, tgLogoBayrak, tgLogoDeneme}
	tgMenuBayrak, tgMenuDeneme = filepath.Join(d, "menu"), filepath.Join(d, "menu-deneme")
	tgLogoBayrak, tgLogoDeneme = filepath.Join(d, "logo"), filepath.Join(d, "logo-deneme")
	t.Cleanup(func() {
		tgTaban = eski
		tgMenuBayrak, tgMenuDeneme, tgLogoBayrak, tgLogoDeneme = eskiYol[0], eskiYol[1], eskiYol[2], eskiYol[3]
		srv.Close()
	})
	return s
}

func (s *sahteTg) say(metod string) int {
	s.mu.Lock()
	defer s.mu.Unlock()
	n := 0
	for _, c := range s.cagri {
		if c == metod {
			n++
		}
	}
	return n
}

func (s *sahteTg) sonForm(metod string) url.Values {
	s.mu.Lock()
	defer s.mu.Unlock()
	f := s.form[metod]
	if len(f) == 0 {
		return url.Values{}
	}
	return f[len(f)-1]
}

func TestTgKlavyeEslesme(t *testing.T) {
	n := 0
	for _, sat := range tgKlavye {
		for _, d := range sat {
			if tgDugmeKomut(d.Metin) != d.Komut || d.Komut == "" {
				t.Fatalf("%q -> %q", d.Metin, tgDugmeKomut(d.Metin))
			}
			n++
		}
	}
	if n != 12 || tgDugmeKomut("merhaba") != "" || tgDugmeKomut("  📷 Foto ") != "/foto" {
		t.Fatalf("eşleşme: %d", n)
	}
}

func TestTgKomutAdlari(t *testing.T) {
	ad := regexp.MustCompile(`^[a-z0-9_]{1,32}$`)
	for _, k := range tgKomutlar {
		if !ad.MatchString(k[0]) || utf8.RuneCountInString(k[1]) < 3 || utf8.RuneCountInString(k[1]) > 256 {
			t.Fatalf("geçersiz komut %q %q", k[0], k[1])
		}
	}
	if len(tgKomutlar) > 100 {
		t.Fatal("çok komut")
	}
}

func TestTgKurulum(t *testing.T) {
	s := yeniSahteTg(t)
	tgKurulumYap(context.Background(), "J", "42")
	for _, m := range []string{"setMyCommands", "setChatMenuButton", "setMyShortDescription", "setMyDescription", "sendMessage"} {
		if s.say(m) != 1 {
			t.Fatalf("%s %d kez", m, s.say(m))
		}
	}
	var komutlar []map[string]string
	if err := json.Unmarshal([]byte(s.sonForm("setMyCommands").Get("commands")), &komutlar); err != nil || len(komutlar) != len(tgKomutlar) {
		t.Fatalf("komutlar %v %d", err, len(komutlar))
	}
	if s.sonForm("setChatMenuButton").Get("menu_button") != `{"type":"commands"}` || s.sonForm("setChatMenuButton").Get("chat_id") != "42" {
		t.Fatal("menü düğmesi")
	}
	var k struct {
		Keyboard     [][]map[string]string `json:"keyboard"`
		IsPersistent bool                  `json:"is_persistent"`
	}
	if err := json.Unmarshal([]byte(s.sonForm("sendMessage").Get("reply_markup")), &k); err != nil || !k.IsPersistent || len(k.Keyboard) != 5 { // v50: Gezi satiri
		t.Fatalf("klavye %v %+v", err, k)
	}
	if !dosyaVar(tgMenuBayrak) {
		t.Fatal("bayrak yok")
	}
	tgKurulumYap(context.Background(), "J", "42")
	if s.say("setMyCommands") != 1 {
		t.Fatal("ikinci kez kuruldu")
	}
}

func TestTgKurulumHata(t *testing.T) {
	s := yeniSahteTg(t)
	s.cevap["setMyCommands"] = `{"ok":false,"description":"Bad Request"}`
	tgKurulumYap(context.Background(), "J", "42")
	if dosyaVar(tgMenuBayrak) || !dosyaVar(tgMenuDeneme) {
		t.Fatal("hatada bayrak yazılmamalı, deneme yazılmalı")
	}
	tgKurulumYap(context.Background(), "J", "42")
	if s.say("setMyCommands") != 1 || s.say("sendMessage") != 1 {
		t.Fatal("1 saat dolmadan yeniden denendi (her 15 sn mesaj yağardı)")
	}
}

func TestTgLogo(t *testing.T) {
	s := yeniSahteTg(t)
	if err := tgLogoAyarla(context.Background(), "J", false); err != nil {
		t.Fatal(err)
	}
	if s.logoJSON != `{"type":"static","photo":"attach://logo"}` || len(s.logo) < 1000 || s.logo[0] != 0xFF || s.logo[1] != 0xD8 ||
		string(s.logo) != string(koleLogo) || !dosyaVar(tgLogoBayrak) {
		t.Fatalf("logo %q %d", s.logoJSON, len(s.logo))
	}
	if err := tgLogoAyarla(context.Background(), "J", false); err != nil || s.say("setMyProfilePhoto") != 1 {
		t.Fatal("bayrak varken yeniden yüklendi")
	}
	_ = os.Remove(tgLogoBayrak)
	_ = os.Remove(tgLogoDeneme)
	s.cevap["setMyProfilePhoto"] = `{"ok":false,"description":"method not found"}`
	if err := tgLogoAyarla(context.Background(), "J", false); err == nil || dosyaVar(tgLogoBayrak) {
		t.Fatal("hata dönmeli, bayrak yazılmamalı")
	}
	_ = tgLogoAyarla(context.Background(), "J", false)
	if s.say("setMyProfilePhoto") != 2 {
		t.Fatal("6 saat dolmadan yeniden denendi")
	}
	_ = tgLogoAyarla(context.Background(), "J", true)
	if s.say("setMyProfilePhoto") != 3 {
		t.Fatal("zorla denenmedi")
	}
}

func satirIci(t *testing.T, f url.Values) [][]map[string]string {
	var m struct {
		Inline [][]map[string]string `json:"inline_keyboard"`
	}
	if err := json.Unmarshal([]byte(f.Get("reply_markup")), &m); err != nil {
		t.Fatalf("reply_markup %v %q", err, f.Get("reply_markup"))
	}
	return m.Inline
}

func TestTgMenuDugmeleri(t *testing.T) {
	s := yeniSahteTg(t)
	ctx := context.Background()
	for _, k := range []string{"#bekci", "#zaman", "#cerceve", "#haber", "#sabah", "#ses", "/menu"} {
		if !tgMenuKomut(ctx, "J", "42", k, "") {
			t.Fatalf("%s işlenmedi", k)
		}
		f := s.sonForm("sendMessage")
		if k == "/menu" {
			if !strings.Contains(f.Get("reply_markup"), `"keyboard"`) {
				t.Fatal("menü klavyesi yok")
			}
			continue
		}
		for _, sat := range satirIci(t, f) {
			for _, d := range sat {
				if cd := d["callback_data"]; len(cd) > 64 || (cd == "" && !strings.HasPrefix(d["url"], merkezAdres)) {
					t.Fatalf("%s: geçersiz düğme %v", k, d)
				}
			}
		}
	}
	tgMenuKomut(ctx, "J", "42", "🛡 Bekçi", "")
	if g := s.sonForm("sendMessage").Get("reply_markup"); !strings.Contains(g, `"k:/bekci ac"`) || !strings.Contains(g, `"k:/bekci kapat"`) {
		t.Fatalf("bekçi düğmeleri %s", g)
	}
	tgMenuKomut(ctx, "J", "42", "/bekci", "")
	if g := s.sonForm("sendMessage").Get("reply_markup"); !strings.Contains(g, `"k:/bekci ac"`) {
		t.Fatal("argümansız /bekci düğme göstermeli")
	}
	if tgDugmeKomut("🎥 Canlı izle") != "/canli" { // v30: düğme Telegram içi canlıyı başlatır (canli_tg_test.go)
		t.Fatal("canlı düğmesi /canli değil")
	}
	if tgMenuKomut(ctx, "J", "42", "/ses merhaba", "") || tgMenuKomut(ctx, "J", "42", "/bekci ac", "") ||
		tgMenuKomut(ctx, "J", "42", "dolar kaç", "") {
		t.Fatal("eski komutlar menü katmanında yutulmamalı")
	}
}

func TestTgDinleDongu(t *testing.T) {
	s := yeniSahteTg(t)
	s.guncel = []string{`[
		{"update_id": 10, "callback_query": {"id": "cb1", "data": "k:/menu", "from": {"id": 42}, "message": {"chat": {"id": 42}}}},
		{"update_id": 11, "message": {"message_id": 5, "text": "/foto", "chat": {"id": 999}}},
		{"update_id": 12, "callback_query": {"id": "cb2", "data": "k:/menu", "from": {"id": 999}, "message": {"chat": {"id": 999}}}},
		{"update_id": 13, "message": {"message_id": 6, "text": "❓ Yardım", "chat": {"id": 42}}}
	]`}
	var kaydedilen int64
	ctx, iptal := context.WithTimeout(context.Background(), 2*time.Second)
	defer iptal()
	ofs, err := tgDinleDongu(ctx, "J", "42", 9, "", func(o int64) { kaydedilen = o })
	if err != nil || ofs != 13 || kaydedilen != 13 {
		t.Fatalf("ofs %d %d %v", ofs, kaydedilen, err)
	}
	if s.say("answerCallbackQuery") != 1 || s.sonForm("answerCallbackQuery").Get("callback_query_id") != "cb1" {
		t.Fatal("yalnız sahibin düğmesi yanıtlanmalı")
	}
	if s.say("sendMessage") != 2 {
		t.Fatalf("sendMessage %d (menü + yardım bekleniyordu; yabancı yok sayılmalı)", s.say("sendMessage"))
	}
	if !strings.Contains(s.sonForm("sendMessage").Get("text"), "TEK DOKUNUŞ") {
		t.Fatal("❓ Yardım düğmesi yardım metnini göndermeli")
	}
	if s.say("getUpdates") != 1 || s.sonForm("getUpdates").Get("timeout") != "0" || s.sonForm("getUpdates").Get("offset") != "10" {
		t.Fatalf("kısa süreli bağlamda uzun sorgu yapılmamalı: %d", s.say("getUpdates"))
	}
	// uzun sorgu: yeterli süre varken 2 ek tur, her biri tgUzunBekleme
	eski := tgUzunBekleme
	tgUzunBekleme = 1
	defer func() { tgUzunBekleme = eski }()
	ctx2, iptal2 := context.WithTimeout(context.Background(), 40*time.Second)
	defer iptal2()
	if _, err := tgDinleDongu(ctx2, "J", "42", 13, "", func(int64) {}); err != nil {
		t.Fatal(err)
	}
	if s.say("getUpdates") != 4 || s.sonForm("getUpdates").Get("timeout") != "1" ||
		!strings.Contains(s.sonForm("getUpdates").Get("allowed_updates"), "callback_query") {
		t.Fatalf("uzun sorgu: %d çağrı, timeout %s", s.say("getUpdates"), s.sonForm("getUpdates").Get("timeout"))
	}
}

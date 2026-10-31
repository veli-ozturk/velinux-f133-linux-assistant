// tg_tek_test.go (23.10.2026, Claude; kole-gemini v41): tek atış --tg-isle çekirdeği, hazır borusu ve canlı izlemenin
// süreçler arası denetimi (sahte Telegram sunucusu canliKur; gerçek Telegram'a dokunmaz).
package main

import (
	"context"
	"io"
	"os"
	"strconv"
	"strings"
	"syscall"
	"testing"
	"time"
)

func TestTgIsleIcYardim(t *testing.T) {
	c, _ := canliKur(t)
	err := tgIsleIc(context.Background(), "J", "42", []byte(`{"update_id":5,"message":{"message_id":9,"text":"/yardim","chat":{"id":42}}}`), "")
	sm := c.son("sendMessage")
	if err != nil || c.say("sendMessage") != 1 || sm["chat_id"] != "42" || !strings.Contains(sm["text"], "VeLinux") {
		t.Fatalf("yardım: %v %v %v", err, c.cagri, sm)
	}
}

func TestTgIsleIcYabanciVeBozuk(t *testing.T) {
	c, _ := canliKur(t)
	if err := tgIsleIc(context.Background(), "J", "42", []byte(`{"update_id":6,"message":{"text":"/yardim","chat":{"id":43}}}`), ""); err != nil {
		t.Fatal(err)
	}
	if err := tgIsleIc(context.Background(), "J", "42", []byte(`{"update_id":`), ""); err == nil {
		t.Fatal("bozuk JSON hata vermedi")
	}
	if len(c.cagri) != 0 {
		t.Fatalf("yabancı sohbet ya da bozuk JSON Telegram'a gitti: %v", c.cagri)
	}
}

func TestTgIsleIcGeriCagri(t *testing.T) {
	c, _ := canliKur(t)
	u := `{"update_id":7,"callback_query":{"id":"cb1","data":"k:/yardim","from":{"id":42},"message":{"chat":{"id":42}}}}`
	if err := tgIsleIc(context.Background(), "J", "42", []byte(u), ""); err != nil {
		t.Fatal(err)
	}
	if c.say("answerCallbackQuery") != 1 || c.son("answerCallbackQuery")["callback_query_id"] != "cb1" || c.say("sendMessage") != 1 {
		t.Fatalf("geri çağrı: %v", c.cagri)
	}
}

func TestTgHazirBildir(t *testing.T) {
	var p [2]int
	if err := syscall.Pipe(p[:]); err != nil {
		t.Fatal(err)
	}
	r := os.NewFile(uintptr(p[0]), "r")
	defer r.Close()
	t.Setenv(tgHazirFd, strconv.Itoa(p[1]))
	tgHazirBildir()
	b, err := io.ReadAll(r) // yazma ucu kapandı: EOF
	if err != nil || string(b) != "hazir\n" {
		t.Fatalf("hazır borusu: %q %v", b, err)
	}
	if os.Getenv(tgHazirFd) != "" {
		t.Fatal("ortam değişkeni silinmedi (ikinci kez yazılmamalı)")
	}
	tgHazirBildir() // ortam yok: hiçbir şey
}

func TestCanliBaskaSurecDenetim(t *testing.T) {
	c, _ := canliKur(t)
	_ = os.WriteFile(canliPid, []byte(strconv.Itoa(os.Getppid())+"\n"), 0644) // yaşayan başka süreç (go test'in üstü)
	if !canliBaskaSurec() {
		t.Fatal("başka süreç görülmedi")
	}
	for _, d := range [][2]string{{"dur", "dur"}, {"uzat", "uzat"}, {"+60", "uzat"}, {"", "yeniden"}} {
		tgCanli(context.Background(), "J", "42", d[0])
		b, _ := os.ReadFile(canliKontrol)
		if strings.TrimSpace(string(b)) != d[1] || canliSurdu() || c.say("sendPhoto") != 0 {
			t.Fatalf("%q -> %q (beklenen %q), sürüyor=%v, çağrı %v", d[0], b, d[1], canliSurdu(), c.cagri)
		}
	}
	_ = os.WriteFile(canliPid, []byte("999999999\n"), 0644) // ölü süreç
	if canliBaskaSurec() {
		t.Fatal("ölü pid başka süreç sayıldı")
	}
	canliPidTemizle()
	if dosyaVar(canliPid) || dosyaVar(canliKontrol) {
		t.Fatal("ölü oturum dosyaları temizlenmedi")
	}
}

func TestCanliKontrolDosyasi(t *testing.T) {
	c, _ := canliKur(t)
	defer sahteKamera(t)()
	tgCanli(context.Background(), "J", "42", "")
	if !canliBekle(t, func() bool { return c.say("sendPhoto") == 1 }, 3*time.Second) {
		t.Fatal("başlamadı")
	}
	if b, _ := os.ReadFile(canliPid); strings.TrimSpace(string(b)) != strconv.Itoa(os.Getpid()) {
		t.Fatalf("pid dosyası: %q", b)
	}
	once := canliBitis()
	if err := canliKontrolYaz("uzat"); err != nil {
		t.Fatal(err)
	}
	if !canliBekle(t, func() bool { return canliBitis().Sub(once) > canliSure/2 }, 2*time.Second) {
		t.Fatalf("dosyadan uzat çalışmadı: %v", canliBitis().Sub(once))
	}
	if err := canliKontrolYaz("dur"); err != nil {
		t.Fatal(err)
	}
	if !canliBekle(t, func() bool { return !canliSurdu() }, 2*time.Second) {
		t.Fatal("dosyadan dur çalışmadı")
	}
	if dosyaVar(canliPid) || dosyaVar(canliKontrol) {
		t.Fatal("oturum bitince pid/denetim dosyası kaldı")
	}
	if ec := c.son("editMessageCaption"); ec == nil || !strings.Contains(ec["caption"], "durduruldu") {
		t.Fatalf("bitiş açıklaması: %v", ec)
	}
}

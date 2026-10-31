// ontampon_test.go (21.10.2026, Claude; kole-gemini v24): ön tampon formülü + gerçek zamanlı sahte köprüyle takılma.
package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"
)

func TestOnTamponFormul(t *testing.T) {
	ms := time.Millisecond
	durumlar := []struct {
		tampon, ilk int
		gecen       time.Duration
		tahmin      int
		ac          bool
	}{
		{200, 200, 300 * ms, 2400, false},   // yalnız ilk parça: hız yok, bekle
		{400, 200, 100 * ms, 2400, true},    // hız 2.0: hemen
		{1000, 200, 1600 * ms, 2400, false}, // hız 0.5: gerekli (1200-500)*1.15+300 = 1105
		{1200, 200, 2000 * ms, 2400, true},
		{300, 200, 4 * time.Second, 2400, true}, // en çok 4 sn
		{2400, 200, 500 * ms, 2400, true},       // tamamı geldi
	}
	for i, d := range durumlar {
		if ac, _ := onTamponYeter(d.tampon, d.ilk, d.gecen, d.tahmin); ac != d.ac {
			t.Fatalf("%d: %+v -> %v", i, d, ac)
		}
	}
	akisOnTamponKapali = true
	ac, _ := onTamponYeter(200, 200, 0, 2400)
	akisOnTamponKapali = false
	if !ac {
		t.Fatal("kapalıyken v23 davranışı: hemen aç")
	}
}

// gerçek zamanlı köprü benzetimi: 150 ms ön tampon dolunca 16 kHz'te çalar; çalma konumu gelen sesi geçerse xrun
type rtKopru struct {
	l    net.Listener
	mu   sync.Mutex
	xrun int
}

func yeniRtKopru(t *testing.T) *rtKopru {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	k := &rtKopru{l: l}
	go func() {
		for {
			c, err := l.Accept()
			if err != nil {
				return
			}
			k.isle(c)
		}
	}()
	t.Cleanup(func() { l.Close() })
	return k
}

func (k *rtKopru) isle(c net.Conn) {
	defer c.Close()
	br := bufio.NewReader(c)
	if _, err := br.ReadString('\n'); err != nil {
		return
	}
	_, _ = c.Write([]byte("PLAY_AKIS HAZIR 16000\n"))
	toplam, bek, basOrnek, xrun := 0, 0, 0, 0
	calis := false
	var basT time.Time
	for {
		h := make([]byte, 2)
		if _, err := io.ReadFull(br, h); err != nil {
			return
		}
		m := int(binary.LittleEndian.Uint16(h))
		if m == 0 {
			break
		}
		if _, err := io.ReadFull(br, make([]byte, m)); err != nil {
			return
		}
		n, simdi := m/2, time.Now()
		if calis && basOrnek+int(simdi.Sub(basT).Seconds()*16000) > toplam {
			xrun++ // bu parça gelmeden tampon boşaldı
			calis, bek = false, 0
		}
		toplam += n
		if !calis {
			bek += n
			if bek >= 2400 {
				calis, basT, basOrnek = true, simdi, toplam-bek
			}
		}
	}
	k.mu.Lock()
	k.xrun += xrun
	k.mu.Unlock()
	fmt.Fprintf(c, "OK %d %d\n", toplam, xrun)
}

// yavaş akış: 12 x 200 ms ses, 400 ms arayla (üretim 0.5x gerçek zaman)
func yavasTTS() *httptest.Server {
	return httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "text/event-stream")
		fl := w.(http.Flusher)
		for i := 0; i < 12; i++ {
			fmt.Fprintf(w, "data: {\"event_type\":\"step.delta\",\"delta\":{\"type\":\"audio\",\"mime_type\":\"audio/l16;rate=24000\",\"data\":\"%s\"}}\n\n",
				pcmB64(sinus(24000, 440, 4800, 3000)))
			fl.Flush()
			if i < 11 {
				time.Sleep(400 * time.Millisecond)
			}
		}
	}))
}

func akisXrun(t *testing.T, kapali bool) (int, string) {
	_, anahtar := akisOrtam(t)
	eskiListe := ttsGeminiModelleri
	ttsGeminiModelleri = nil
	akisOnTamponKapali = kapali
	t.Cleanup(func() { ttsGeminiModelleri = eskiListe; akisOnTamponKapali = false })
	k := yeniRtKopru(t)
	kopruAdres = k.l.Addr().String()
	srv := yavasTTS()
	defer srv.Close()
	ttsURL = srv.URL
	t.Setenv("KOLE_KONUS_METIN", strings.Repeat("a", 35)) // tahmin 35 x 69 = 2415 ms (gerçek 2400 ms)
	if rc := akisCalistir(anahtar); rc != 0 {
		g, _ := os.ReadFile(ttsGunlukYol)
		t.Fatalf("rc %d\n%s", rc, g)
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	m := regexp.MustCompile(`xrun (\d+)`).FindStringSubmatch(string(g))
	if m == nil {
		t.Fatalf("günlükte xrun yok:\n%s", g)
	}
	x, _ := strconv.Atoi(m[1])
	return x, string(g)
}

func TestAkisOnTamponGercekZaman(t *testing.T) {
	eski, g := akisXrun(t, true)
	if eski == 0 {
		t.Fatalf("v23 davranışı (ilk parçada aç) 0.5x akışta takılmalıydı:\n%s", g)
	}
	yeni, g := akisXrun(t, false)
	if yeni != 0 || !strings.Contains(g, "ön tampon") {
		t.Fatalf("v24 ön tamponla xrun %d (v23: %d):\n%s", yeni, eski, g)
	}
	t.Logf("0.5x akış: v23 xrun %d -> v24 xrun %d", eski, yeni)
}

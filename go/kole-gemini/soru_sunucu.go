// soru_sunucu.go (23.10.2026, Claude; kole-gemini v40, madde 131): sürekli --telegram süreci yerel Unix soketinde soru
// alır; Hey Köle (kole-asistan v18 doğrudan, eskisi --ses ince istemciyle) ve panel/Telegram --metin soruları yeni Go süreci
// açmadan cevaplanır (ölçüm: soru başına ~3.7 MB anonim + ses alt süreci). Protokol: istemci tek satır JSON yazar
// {"ses":"/tmp/x.wav"} ya da {"metin":"..."}; sunucu tek satır yazıp kapatır: sonuc JSON ya da {"rc":N}.
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"net"
	"os"
	"strings"
	"sync"
	"time"
)

var (
	soruSoket    = "/tmp/kole-gemini.sock"
	soruMu       sync.Mutex     // sorular sırayla (butce/cikisSiniri/geçmiş paylaşımlı)
	sunucuYollar soruYollar     // main --telegram'da ayarlar; boşsa sunucu açılmaz
	arkaIs       sync.WaitGroup // süren istek + ses akışı: süreç çıkmadan beklenir
)

func soruIsleKilitli(y soruYollar, ist soruIstek, alindi time.Time) (sonuc, int) {
	soruMu.Lock()
	defer soruMu.Unlock()
	if sesBayat(ist, alindi) { // v43 (A20)
		fmt.Fprintln(os.Stderr, "soru soketi: ses dosyası istekten sonra değişmiş (bayat istek), atlandı")
		return sonuc{}, 5
	}
	y.ayriSurec = false
	return soruIsle(y, ist)
}

// soruSunucusu: dinlemeye başlar; dönen işlev dinlemeyi kapatır ve süren işleri en çok 90 sn bekler
func soruSunucusu(y soruYollar) func() {
	if y.anahtar == "" {
		return func() {}
	}
	_ = os.Remove(soruSoket)
	ln, err := net.Listen("unix", soruSoket)
	if err != nil {
		fmt.Fprintln(os.Stderr, "soru soketi:", err)
		return func() {}
	}
	_ = os.Chmod(soruSoket, 0600)
	go func() {
		for {
			c, err := ln.Accept()
			if err != nil {
				return
			}
			arkaIs.Add(1)
			go func() {
				defer arkaIs.Done()
				soruBaglanti(c, y)
			}()
		}
	}()
	return func() {
		_ = ln.Close() // Listen ile açılan Unix soketi kapanınca dosyası da silinir
		bitti := make(chan struct{})
		go func() { arkaIs.Wait(); close(bitti) }()
		select {
		case <-bitti:
		case <-time.After(90 * time.Second):
			fmt.Fprintln(os.Stderr, "soru sunucusu: süren iş 90 sn'de bitmedi")
		}
	}
}

func soruBaglanti(c net.Conn, y soruYollar) {
	defer c.Close()
	defer func() {
		if r := recover(); r != nil {
			fmt.Fprintln(os.Stderr, "soru sunucusu panik:", r)
			_, _ = c.Write([]byte("{\"rc\":70}\n"))
		}
	}()
	_ = c.SetReadDeadline(time.Now().Add(5 * time.Second))
	sat, err := bufio.NewReader(io.LimitReader(c, 64<<10)).ReadString('\n')
	var ist soruIstek
	if err != nil || json.Unmarshal([]byte(sat), &ist) != nil || (ist.Ses == "") == (ist.Metin == "") {
		_, _ = c.Write([]byte("{\"rc\":2}\n"))
		return
	}
	t0 := time.Now()
	s, rc := soruIsleKilitli(y, ist, t0)
	kip := "metin"
	if ist.Ses != "" {
		kip = "ses"
	}
	fmt.Fprintf(os.Stderr, "%s soru soketi (%s): rc %d, %d ms\n", time.Now().Format("15:04:05"), kip, rc, time.Since(t0).Milliseconds())
	_ = c.SetWriteDeadline(time.Now().Add(5 * time.Second))
	if rc != 0 {
		_, _ = fmt.Fprintf(c, "{\"rc\":%d}\n", rc)
		return
	}
	b, _ := json.Marshal(s)
	_, _ = c.Write(append(b, '\n'))
}

// soketSor: ince istemci. ok=false: sunucu yok (bağlanılamadı ya da istek gönderilemedi) -> çağıran kendisi yapar.
// ok=true: cevap ya da rc (sunucu cevapsız kapanırsa rc 5).
func soketSor(yol string, ist soruIstek, bekle time.Duration) (string, int, bool) {
	c, err := net.DialTimeout("unix", yol, time.Second)
	if err != nil {
		return "", 0, false
	}
	defer c.Close()
	_ = c.SetDeadline(time.Now().Add(bekle))
	b, _ := json.Marshal(ist)
	if _, err := c.Write(append(b, '\n')); err != nil {
		return "", 0, false
	}
	sat, _ := bufio.NewReader(c).ReadString('\n')
	sat = strings.TrimSpace(sat)
	if sat == "" {
		return "", 5, true
	}
	var r struct {
		Rc *int `json:"rc"`
	}
	if json.Unmarshal([]byte(sat), &r) == nil && r.Rc != nil {
		return "", *r.Rc, true
	}
	return sat, 0, true
}

// sesBayat: v43 (denetim v3 A20): sorular sırayla işlenir. İstemci (kole-asistan ya da ince istemci) zaman aşımıyla
// vazgeçip yeni soru kaydettiyse sabit /tmp/asistan-soru.wav artık başka sorunun kaydıdır: istek alındıktan sonra
// değişmiş ses dosyası bayattır (yeni soru kendi isteğiyle gelir).
func sesBayat(ist soruIstek, alindi time.Time) bool {
	if ist.Ses == "" {
		return false
	}
	st, err := os.Stat(ist.Ses)
	return err == nil && st.ModTime().After(alindi)
}

// canli_aralik_test.go (23.10.2026, Claude; v36): aralık yüklemenin BAŞINDAN sayılır.
package main

import (
	"context"
	"testing"
	"time"
)

var canliTestGecikme time.Duration // canliKur sunucusu editMessageMedia'yı bu kadar geciktirir

func TestCanliAralikBastan(t *testing.T) {
	c, _ := canliKur(t)
	defer sahteKamera(t)()
	canliSure, canliAralik = 5*time.Second, 400*time.Millisecond
	canliTestGecikme = 300 * time.Millisecond
	defer func() { canliTestGecikme = 0 }()
	tgCanli(context.Background(), "J", "42", "")
	if !canliBekle(t, func() bool { return c.say("editMessageMedia") >= 1 }, 4*time.Second) {
		t.Fatal("düzenleme başlamadı")
	}
	n0 := c.say("editMessageMedia")
	time.Sleep(2400 * time.Millisecond)
	n := c.say("editMessageMedia") - n0
	tgCanli(context.Background(), "J", "42", "dur")
	canliBekle(t, func() bool { return !canliSurdu() }, 3*time.Second)
	// 400 ms aralık yüklemenin başından: ~6; v35 yolu (yükleme 300 + bekleme 400 = 700 ms): ~3
	if n < 5 || n > 7 {
		t.Fatalf("2.4 sn'de %d düzenleme (beklenen 5..7)", n)
	}
}

func TestCanliAralikVarsayilan(t *testing.T) {
	if canliAralik != time.Second || canliEnAz >= canliAralik {
		t.Fatalf("aralık %v, en az %v", canliAralik, canliEnAz)
	}
}

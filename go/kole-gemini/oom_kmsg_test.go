// oom_kmsg_test.go (23.10.2026, Claude; v37): gerçek 23.10 dmesg kayıtlarıyla kurban/tetikleyen ve mesaj.
package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

const oomOrnek = "4,9001,19317023364,-;sh invoked oom-killer: gfp_mask=0x400dc0(GFP_KERNEL_ACCOUNT|__GFP_ZERO), order=2, oom_score_adj=0\n" +
	"6,9002,19317024619,-;oom-kill:constraint=CONSTRAINT_NONE,nodemask=(null),task=kole-gemini-v19,pid=12062,uid=0\n" +
	"3,9003,19317024660,-;Out of memory: Killed process 12062 (kole-gemini-v19) total-vm:1266688kB, anon-rss:0kB, file-rss:4kB\n" +
	"6,9004,20000000000,-;wlan0: baska bir satir\n" +
	"4,9101,22898622094,-;ssv6xxx_tx_task invoked oom-killer: gfp_mask=0xc2cc0(GFP_KERNEL|__GFP_NOWARN|__GFP_COMP|__GFP_NOMEMALLOC), order=3, oom_score_adj=0\n" +
	"3,9102,22898623025,-;Out of memory: Killed process 14417 (kole-gemini-v19) total-vm:1266688kB, anon-rss:920kB\n"

func TestOomSatirlari(t *testing.T) {
	o := oomSatirlari(strings.Split(strings.TrimSpace(oomOrnek), "\n"))
	if len(o) != 2 || o[0] != (oomOlay{"kole-gemini-v19", "sh", 2}) || o[1] != (oomOlay{"kole-gemini-v19", "ssv6xxx_tx_task", 3}) {
		t.Fatalf("%+v", o)
	}
	if o := oomSatirlari([]string{"Out of memory: Killed process 5 (panel) total-vm:1kB"}); len(o) != 1 || o[0].Tetik != "" || o[0].Derece != -1 {
		t.Fatalf("tetiksiz: %+v", o)
	}
}

func TestOomOlayOkuDosya(t *testing.T) {
	eski := kmsgYol
	kmsgYol = filepath.Join(t.TempDir(), "kmsg")
	defer func() { kmsgYol = eski }()
	if oomOlayOku(1) != nil {
		t.Fatal("dosya yokken nil olmalı")
	}
	// düz dosyada tek read tüm içeriği getirir; kmsg'de her read bir kayıt. İkisi de çalışmalı.
	os.WriteFile(kmsgYol, []byte(oomOrnek), 0644)
	o := oomOlayOku(1)
	if len(o) != 1 || o[0].Tetik != "ssv6xxx_tx_task" || o[0].Derece != 3 {
		t.Fatalf("son olay: %+v", o)
	}
	if o := oomOlayOku(5); len(o) != 2 {
		t.Fatalf("hepsi: %+v", o)
	}
	if oomOlayOku(0) != nil {
		t.Fatal("n=0")
	}
}

func TestOomMesaji(t *testing.T) {
	m := oomMesaji(1, []oomOlay{{"kole-gemini-v19", "ssv6xxx_tx_task", 3}})
	for _, p := range []string{"ℹ️", "1 süreci kapattı (OOM)", "Telegram/Hey Köle istemcisi (kole-gemini-v19)",
		"Wi-Fi sürücüsü 32 KB bitişik blok istedi", "işlem gerekmez"} {
		if !strings.Contains(m, p) {
			t.Fatalf("%q yok: %s", p, m)
		}
	}
	m = oomMesaji(2, []oomOlay{{"kole-gemini-v19", "sh", 2}, {"panel", "sh", 2}})
	if !strings.HasPrefix(m, "⚠️") || !strings.Contains(m, "panel (panel)") || strings.Count(m, "sh süreci 16 KB") != 1 ||
		!strings.Contains(m, "/saglik") {
		t.Fatal(m)
	}
	if m := oomMesaji(1, nil); !strings.Contains(m, "Bellek yetmedi: çekirdek 1 süreci kapattı (OOM)") {
		t.Fatal(m)
	}
	if oomSurecAdi("kole-cihaz-merk") != "KÖLE Merkez" || oomSurecAdi("tailscaled-kucu") != "uzaktan erişim (Tailscale)" ||
		oomSurecAdi("xyz") != "xyz" {
		t.Fatal("adlar")
	}
}

func TestSayacOomKurban(t *testing.T) {
	eski := oomOlayOku
	oomOlayOku = func(n int) []oomOlay { return []oomOlay{{"kole-gemini-v19", "sh", 2}} }
	defer func() { oomOlayOku = eski }()
	var d sayacDurum
	sayacAdimi(&d, sayaclar{MemKB: 30000, Oom: 2, Zaman: 1000}, 12, 200, "g")
	m := sayacAdimi(&d, sayaclar{MemKB: 30000, Oom: 3, Zaman: 1060}, 12, 200, "g")
	if len(m) != 1 || !strings.Contains(m[0], "kole-gemini-v19") || !strings.Contains(m[0], "1 süreci") {
		t.Fatalf("%v", m)
	}
}

// saglik_test.go (15.10.2026, Claude): saglik.go saf kararları ve sahte /proc ile hizmet taraması.
package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestHizmetTara(t *testing.T) {
	d := t.TempDir()
	eski := procKok
	procKok = d
	defer func() { procKok = eski }()
	yaz := func(pid, comm string) {
		_ = os.MkdirAll(filepath.Join(d, pid), 0755)
		_ = os.WriteFile(filepath.Join(d, pid, "comm"), []byte(comm+"\n"), 0644)
	}
	yaz("10", "panel")
	yaz("11", "kamera-kayit-v7")
	yaz("12", "tailscaled-kucu")
	yaz("13", "panelx")
	yaz("14", "kole-cihaz-merk")
	_ = os.MkdirAll(filepath.Join(d, "self"), 0755)
	hz := hizmetTara()
	if hz["panel"] != "panel" || hz["kamera_kayit"] != "kamera-kayit-v7" || hz["tailscale"] != "tailscaled-kucu" || hz["merkez"] != "kole-cihaz-merk" {
		t.Fatalf("tarama: %v", hz)
	}
	if hz["asistan"] != "" || hz["kole_kopru"] != "" {
		t.Fatalf("olmayan hizmet bulundu: %v", hz)
	}
	_ = os.WriteFile(filepath.Join(d, "meminfo"), []byte("MemTotal: 56372 kB\nMemAvailable:   20480 kB\n"), 0644)
	if memAvailableKB() != 20480 {
		t.Fatal("meminfo")
	}
}

func TestSabahKarari(t *testing.T) {
	loc := time.FixedZone("TR", 3*3600)
	g := func(s string) time.Time { v, _ := time.ParseInLocation("2006-01-02 15:04", s, loc); return v }
	durumlar := []struct {
		an   string
		son  string
		bekl bool
	}{
		{"2026-10-16 08:29", "", false},
		{"2026-10-16 08:30", "", true},
		{"2026-10-16 10:00", "2026-10-15", true},
		{"2026-10-16 10:00", "2026-10-16", false},
		{"2026-10-16 11:30", "", false},
		{"2026-10-16 17:00", "", false},
	}
	for _, d := range durumlar {
		if sabahGonderilsinMi(g(d.an), 8, 30, d.son) != d.bekl {
			t.Errorf("%s son=%q: beklenen %v", d.an, d.son, d.bekl)
		}
	}
}

func TestUyariDurumMakinesi(t *testing.T) {
	var d uyariDurum
	tam := map[string]string{"panel": "panel", "kamera_kayit": "kamera-kayit-v7", "kole_kopru": "kole-kopru-v9", "merkez": "kole-cihaz-merk",
		"tailscale": "tailscaled-kucu", "asistan": "kole-asistan-v8", "mcu": "mcu-servis-v3"}
	if m := uyariAdimi(&d, tam, false, 1000); len(m) != 0 {
		t.Fatalf("hepsi calisirken mesaj: %v", m)
	}
	eksik := map[string]string{}
	for k, v := range tam {
		if k != "kamera_kayit" {
			eksik[k] = v
		}
	}
	for i := 0; i < 2; i++ {
		if m := uyariAdimi(&d, eksik, false, int64(1015+i*15)); len(m) != 0 {
			t.Fatalf("esikten once uyari: %v", m)
		}
	}
	m := uyariAdimi(&d, eksik, false, 1045)
	if len(m) != 1 || !strings.Contains(m[0], "Kamera kaydı çalışmıyor") {
		t.Fatalf("3. kacirmada uyari bekleniyordu: %v", m)
	}
	if m := uyariAdimi(&d, eksik, false, 1060); len(m) != 0 {
		t.Fatalf("tekrar uyari: %v", m)
	}
	m = uyariAdimi(&d, tam, false, 1075)
	if len(m) != 1 || !strings.Contains(m[0], "yeniden çalışıyor") {
		t.Fatalf("geri gelme mesaji: %v", m)
	}
	// saatte bir: hemen yeniden duserse uyari yok
	for i := 0; i < 3; i++ {
		uyariAdimi(&d, eksik, false, int64(1090+i*15))
	}
	if d.Dustu["kamera_kayit"] {
		t.Fatal("1 saat dolmadan ikinci uyari isaretlendi")
	}
	if m := uyariAdimi(&d, eksik, false, 1045+3600); len(m) != 1 {
		t.Fatalf("1 saat sonra uyari bekleniyordu: %v", m)
	}
	// asistan ayarlardan kapaliysa uyari yok
	var d2 uyariDurum
	asistansiz := map[string]string{}
	for k, v := range tam {
		if k != "asistan" {
			asistansiz[k] = v
		}
	}
	for i := 0; i < 5; i++ {
		if m := uyariAdimi(&d2, asistansiz, true, int64(i*15)); len(m) != 0 {
			t.Fatalf("asistan kapaliyken uyari: %v", m)
		}
	}
}

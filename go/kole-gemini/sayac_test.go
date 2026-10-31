// sayac_test.go (19.10.2026, Claude): sayac.go — sahte /proc ayrıştırma ve saf eşik kararları.
package main

import (
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

func sahteProc(t *testing.T, memKB int64, rxErr, rxDrop, txErr, txDrop uint64, oom int64) {
	t.Helper()
	d := procKok
	_ = os.WriteFile(filepath.Join(d, "meminfo"), []byte("MemTotal: 56372 kB\nMemAvailable: "+itoa(memKB)+" kB\n"), 0644)
	_ = os.MkdirAll(filepath.Join(d, "net"), 0755)
	dev := "Inter-|   Receive                                                |  Transmit\n" +
		" face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed\n" +
		"    lo: 100 2 0 0 0 0 0 0 100 2 0 0 0 0 0 0\n" +
		" wlan0: 123456 789 " + utoa(rxErr) + " " + utoa(rxDrop) + " 0 0 0 0 654321 987 " + utoa(txErr) + " " + utoa(txDrop) + " 0 0 0 0\n"
	_ = os.WriteFile(filepath.Join(d, "net/dev"), []byte(dev), 0644)
	_ = os.WriteFile(filepath.Join(d, "vmstat"), []byte("nr_free_pages 100\noom_kill "+itoa(oom)+"\n"), 0644)
}

func itoa(v int64) string  { return strconv.FormatInt(v, 10) }
func utoa(v uint64) string { return strconv.FormatUint(v, 10) }

func TestSayacOku(t *testing.T) {
	eski := procKok
	procKok = t.TempDir()
	defer func() { procKok = eski }()
	sahteProc(t, 20480, 1, 2, 3, 4, 7)
	s := sayacOku(1000)
	if s.MemKB != 20480 || !s.WifiOK || s.RxErr != 1 || s.RxDrop != 2 || s.TxErr != 3 || s.TxDrop != 4 || s.Oom != 7 || s.Zaman != 1000 {
		t.Fatalf("ayrıştırma: %+v", s)
	}
	_ = os.WriteFile(filepath.Join(procKok, "net/dev"), []byte("x\ny\n    lo: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16\n"), 0644)
	_ = os.Remove(filepath.Join(procKok, "vmstat"))
	s = sayacOku(1001)
	if s.WifiOK || s.Oom != -1 {
		t.Fatalf("wlan0/vmstat yokken: %+v", s)
	}
}

func TestBellekEsigi(t *testing.T) {
	var d sayacDurum
	z := int64(10000)
	adim := func(kb int64) []string {
		z += 60
		return sayacAdimi(&d, sayaclar{MemKB: kb, Oom: 0, Zaman: z}, 12, 200, "2026-10-19")
	}
	if m := adim(10 * 1024); len(m) != 0 {
		t.Fatalf("ilk düşük ölçümde uyarı: %v", m)
	}
	adim(11 * 1024)
	m := adim(9 * 1024)
	if len(m) != 1 || !strings.Contains(m[0], "Boş bellek az") || !strings.Contains(m[0], "9 MB") {
		t.Fatalf("3. düşük ölçümde uyarı bekleniyordu: %v", m)
	}
	if m := adim(8 * 1024); len(m) != 0 {
		t.Fatalf("tekrar uyarı: %v", m)
	}
	if m := adim(14 * 1024); len(m) != 0 {
		t.Fatalf("eşik+4 altında rahatladı: %v", m)
	}
	m = adim(17 * 1024)
	if len(m) != 1 || !strings.Contains(m[0], "rahat") {
		t.Fatalf("rahatlama: %v", m)
	}
	if d.EnDusukKB != 8*1024 {
		t.Fatalf("en düşük: %d", d.EnDusukKB)
	}
	// saat dolmadan yeniden düşerse uyarı yok (saatte en çok bir)
	adim(5 * 1024)
	adim(5 * 1024)
	if m := adim(5 * 1024); len(m) != 0 {
		t.Fatalf("saat dolmadan ikinci uyarı: %v", m)
	}
	// yeni gün: en düşük sıfırlanır
	z += 60
	sayacAdimi(&d, sayaclar{MemKB: 30 * 1024, Oom: 0, Zaman: z}, 12, 200, "2026-10-20")
	if d.EnDusukKB != 30*1024 || d.EnDusukGun != "2026-10-20" {
		t.Fatalf("gün sıfırlama: %+v", d)
	}
}

func TestOomVeWifi(t *testing.T) {
	var d sayacDurum
	z := int64(50000)
	s := sayaclar{MemKB: 30000, WifiOK: true, Oom: 2, Zaman: z}
	if m := sayacAdimi(&d, s, 12, 200, "g"); len(m) != 0 {
		t.Fatalf("ilk ölçüm: %v", m)
	}
	z += 60
	s = sayaclar{MemKB: 30000, WifiOK: true, Oom: 3, Zaman: z}
	m := sayacAdimi(&d, s, 12, 200, "g")
	if len(m) != 1 || !strings.Contains(m[0], "OOM") || !strings.Contains(m[0], "1 süreci") {
		t.Fatalf("OOM uyarısı: %v", m)
	}
	// Wi-Fi: 6 dk boyunca dakikada 60 hata -> 360 > 200: uyarı
	var uyari []string
	for i := 1; i <= 6; i++ {
		z += 60
		s = sayaclar{MemKB: 30000, WifiOK: true, RxDrop: uint64(60 * i), Oom: 3, Zaman: z}
		uyari = append(uyari, sayacAdimi(&d, s, 12, 200, "g")...)
	}
	if len(uyari) != 1 || !strings.Contains(uyari[0], "Wi-Fi") {
		t.Fatalf("Wi-Fi uyarısı tek olmalı: %v", uyari)
	}
	// sürücü yeniden yüklendi: sayaç sıfırlandı -> negatif artış yok, taban yenilenir
	z += 60
	s = sayaclar{MemKB: 30000, WifiOK: true, RxDrop: 5, Oom: 3, Zaman: z}
	sayacAdimi(&d, s, 12, 200, "g")
	if d.Birikim != 360 {
		t.Fatalf("sıfırlanmada birikim: %d", d.Birikim)
	}
	// 11 dk sakin -> azaldı
	var son []string
	for i := 0; i < 11; i++ {
		z += 60
		s = sayaclar{MemKB: 30000, WifiOK: true, RxDrop: 5, Oom: 3, Zaman: z}
		son = append(son, sayacAdimi(&d, s, 12, 200, "g")...)
	}
	if len(son) != 1 || !strings.Contains(son[0], "azaldı") {
		t.Fatalf("Wi-Fi azaldı: %v", son)
	}
	if len(d.Noktalar) > 12 {
		t.Fatalf("pencere budanmadı: %d nokta", len(d.Noktalar))
	}
}

func TestSaglikLog(t *testing.T) {
	eski := saglikLogYol
	saglikLogYol = filepath.Join(t.TempDir(), "saglik.log")
	defer func() { saglikLogYol = eski }()
	satir := strings.Repeat("x", 99) + "\n"
	for i := 0; i < 1000; i++ {
		saglikLogEkle(satir)
	}
	st, err := os.Stat(saglikLogYol)
	if err != nil || st.Size() > saglikLogAzami+200 {
		t.Fatalf("günlük budanmadı: %v %d", err, st.Size())
	}
}

// oom_kmsg_gercek_test.go (23.10.2026, Claude; v37): GERÇEK /dev/kmsg okunabiliyorsa oomOlayOku takılmadan dönmeli
// (O_NONBLOCK + EAGAIN; düz dosya testi bunu sınamaz). Okunamazsa atlanır (normal derlemede root yok).
// WSL'de root ile, önce sahte satır yazılarak (scripts/559):
//
//	echo "Out of memory: Killed process 1 (kole-gemini-v19) deneme" > /dev/kmsg
//	OOM_KMSG_BEKLE=kole-gemini-v19 go test -run TestOomKmsgGercek -v
package main

import (
	"os"
	"syscall"
	"testing"
	"time"
)

func TestOomKmsgGercek(t *testing.T) {
	fd, err := syscall.Open("/dev/kmsg", syscall.O_RDONLY|syscall.O_NONBLOCK, 0)
	if err != nil {
		t.Skip("kmsg okunamıyor:", err)
	}
	syscall.Close(fd)
	bitti := make(chan []oomOlay, 1)
	t0 := time.Now()
	go func() { bitti <- oomOlayOku(3) }()
	select {
	case o := <-bitti:
		t.Logf("gerçek kmsg %v: %d OOM olayı %+v", time.Since(t0).Round(time.Millisecond), len(o), o)
		if b := os.Getenv("OOM_KMSG_BEKLE"); b != "" && (len(o) == 0 || o[len(o)-1].Kurban != b) {
			t.Fatalf("son kurban %q bekleniyordu: %+v", b, o)
		}
	case <-time.After(3 * time.Second):
		t.Fatal("oomOlayOku gerçek kmsg'de takıldı (EAGAIN beklenmiyor mu?)")
	}
}

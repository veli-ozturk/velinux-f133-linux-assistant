// canli_tg.go v4 (23.10.2026, Claude; kole-gemini v42): uzatma sınırı oturum başından (canliDurum.bas + canliAzami);
// v3 her "uzat"ta şimdiden sayıyordu (izleme sınırsız uzuyor, tek atış süreç yalnız canliAzami+30 sn bekliyordu). --- v3 ---
// canli_tg.go v3 (23.10.2026, Claude; kole-gemini v41): süreçler arası denetim. Tek atış --tg-isle sürecinde oturum
// canliPid'e yazar; sonraki süreçlerin "dur" / "uzat" / sürerken "/canli" komutu canliKontrol dosyasıyla gelir.
// --- v2 ---
// canli_tg.go v2 (23.10.2026, Claude; kole-gemini v36, kullanıcı: "telegramdaki canlı izlemeyi her saniyi yenileyecek şekilde"):
// aralık 1 sn ve önceki yüklemenin BAŞINDAN sayılır (v1: yüklemeden SONRA 2 sn, fiilen ~3 sn); yeni kare yoksa 150 ms
// sonra yeniden bakılır; 429 gelirse retry_after + o izlemede 2 sn; izleme sonunda stderr ölçüm satırı.
// --- v1 ---
// canli_tg.go (22.10.2026, Claude; kole-gemini v30, kullanıcı: "telegram botundan canlı izle diyince doğrudan telegram
// üzerinden canlı izleyebileyim"): /canli -> Telegram İÇİNDE canlı kamera. Telegram botları gerçek video akışı veremez;
// tek bir fotoğraf mesajı ~2 sn'de bir editMessageMedia ile yenilenir (sohbette "canlı" görüntü).
//   - Kaynak: kamera-kayit, /tmp/kamera-aktif varken her kareyi /tmp/kamera-son.jpg'ye yazar (~8 kare/sn).
//   - Bayrak "telegram <bitiş unix>" içerikle yazılır, yalnız bizimkiyse silinir (Merkez kendi "merkez" bayrağını aynı
//     kuralla yönetir: birbirinin izlemesini kesmez). Süreç izleme ortasında ölürse süresi geçmiş bayrak açılışta silinir.
//   - Gizlilik kalkanı açıksa başlamaz; izleme sırasında açılırsa durur.
//   - 60 sn; "➕ 60 sn" uzatır (en çok 5 dk), "⏹ Durdur" bitirir; bitince son kare kalır, "🎥 Yeniden izle" düğmesi.
//   - Yeni kare yoksa (kamera durmuşsa) düzenleme yapılmaz; 3 ardışık Telegram hatasında durur (429'da retry_after kadar).
package main

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"os"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
)

var (
	canliBayrak = "/tmp/kamera-aktif"
	canliKare   = "/tmp/kamera-son.jpg"
	canliSure   = 60 * time.Second
	canliAzami  = 5 * time.Minute
	canliAralik = 1 * time.Second        // v36: önceki yüklemenin başından (v30-v35: yüklemeden sonra 2 sn)
	canliEnAz   = 150 * time.Millisecond // v36: yeni kare yoksa yeniden bakma aralığı
	canliIlkBek = 8 * time.Second
	canliKalkan = kalkanBayrak // testte değiştirilir
)

var (
	canliPid     = "/tmp/tg-canli.pid"     // v41: oturumu yürüten süreç
	canliKontrol = "/tmp/tg-canli-kontrol" // v41: başka süreçten "dur" / "uzat" / "yeniden"
)

var canliDurum struct {
	sync.Mutex
	aktif bool
	bas   time.Time // v42: oturum başı (bitiş en çok bas + canliAzami)
	bitis time.Time
	dur   chan struct{}
	kapat *sync.Once
}

func canliSurdu() bool {
	canliDurum.Lock()
	defer canliDurum.Unlock()
	return canliDurum.aktif
}

func canliBitis() time.Time {
	canliDurum.Lock()
	defer canliDurum.Unlock()
	return canliDurum.bitis
}

func canliBayrakYaz(bitis time.Time) {
	if b, err := os.ReadFile(canliBayrak); err == nil && !strings.HasPrefix(string(b), "telegram") {
		return // Merkez/panel izliyor: bayrak onların, dokunma
	}
	_ = os.WriteFile(canliBayrak, []byte(fmt.Sprintf("telegram %d\n", bitis.Unix())), 0644)
}

func canliBayrakSil() {
	if b, err := os.ReadFile(canliBayrak); err == nil && strings.HasPrefix(string(b), "telegram") {
		_ = os.Remove(canliBayrak)
	}
}

// canliEskiTemizle: önceki --telegram süreci izleme ortasında öldüyse kalan (süresi geçmiş) telegram bayrağı
func canliEskiTemizle() {
	b, err := os.ReadFile(canliBayrak)
	if err != nil || !strings.HasPrefix(string(b), "telegram") {
		return
	}
	var t int64
	if _, e := fmt.Sscanf(string(b), "telegram %d", &t); e != nil || time.Now().Unix() > t {
		_ = os.Remove(canliBayrak)
	}
}

// canliKareOku: son kare ve yazılma anı
func canliKareOku() ([]byte, time.Time, bool) {
	st, err := os.Stat(canliKare)
	if err != nil {
		return nil, time.Time{}, false
	}
	b, err := os.ReadFile(canliKare)
	if err != nil || len(b) < 2 || b[0] != 0xFF || b[1] != 0xD8 || len(b) > 5<<20 {
		return nil, time.Time{}, false
	}
	return b, st.ModTime(), true
}

func canliDugmeler(aktif bool) map[string]any {
	if aktif {
		return tgSatirIci(tgSatir(tgK("⏹ Durdur", "/canli dur"), tgK("➕ 60 sn", "/canli uzat")),
			tgSatir(tgU("🏠 Merkez'de akıcı izle", merkezAdres+"/canli")))
	}
	return tgSatirIci(tgSatir(tgK("🎥 Yeniden izle", "/canli"), tgU("🏠 Merkez'de akıcı izle", merkezAdres+"/canli")))
}

func canliBaslik(kalan time.Duration) string {
	return fmt.Sprintf("🎥 Canlı · %s · kalan %d sn", time.Now().Format("15:04:05"), int(kalan.Round(time.Second)/time.Second))
}

// tgCanli: "/canli" (başlat; sürüyorsa uzat), "/canli uzat", "/canli dur"
func tgCanli(ctx context.Context, jeton, sahip, arg string) {
	canliDurum.Lock()
	if !canliDurum.aktif && canliBaskaSurec() { // v41: izleme önceki tek atış sürecinde
		canliDurum.Unlock()
		k := "yeniden"
		switch arg {
		case "dur", "durdur", "kapat":
			k = "dur"
		case "uzat", "+60":
			k = "uzat"
		}
		if err := canliKontrolYaz(k); err != nil {
			fmt.Fprintln(os.Stderr, "canli kontrol:", err)
		}
		return
	}
	switch arg {
	case "dur", "durdur", "kapat":
		if canliDurum.aktif {
			canliDurum.kapat.Do(func() { close(canliDurum.dur) })
		}
		canliDurum.Unlock()
		return
	case "uzat", "+60":
		if canliDurum.aktif {
			canliDurum.bitis = canliSinirla(canliDurum.bitis.Add(canliSure)) // v42
		}
		canliDurum.Unlock()
		return
	}
	if canliDurum.aktif { // sürerken /canli: uzat
		canliDurum.bitis = canliSinirla(time.Now().Add(canliSure)) // v42
		canliDurum.Unlock()
		return
	}
	if dosyaVar(canliKalkan) {
		canliDurum.Unlock()
		telegramYanit(ctx, jeton, sahip, "🛡 Panelde gizlilik kalkanı açık efendim: kamera uzaktan açılamaz.")
		return
	}
	canliDurum.aktif = true
	canliDurum.bas = time.Now() // v42
	canliDurum.bitis = canliDurum.bas.Add(canliSure)
	canliDurum.dur = make(chan struct{})
	canliDurum.kapat = &sync.Once{}
	dur := canliDurum.dur
	canliDurum.Unlock()
	go canliOturum(jeton, sahip, dur)
}

func canliOturum(jeton, sahip string, dur <-chan struct{}) {
	ctx, iptal := context.WithTimeout(context.Background(), canliAzami+2*time.Minute)
	defer iptal()
	defer func() {
		canliBayrakSil()
		canliPidSil() // v41
		canliDurum.Lock()
		canliDurum.aktif = false
		canliDurum.Unlock()
	}()
	canliPidYaz() // v41: başka tek atış süreç "dur"/"uzat"ı dosyadan iletir
	canliBayrakYaz(canliBitis())
	bas := time.Now()
	var kare []byte
	var son time.Time
	for time.Since(bas) < canliIlkBek { // ilk taze kare (kamera-kayit bayrağı görüp yazmaya başlamalı)
		if b, mt, ok := canliKareOku(); ok && mt.After(bas.Add(-2*time.Second)) {
			kare, son = b, mt
			break
		}
		select {
		case <-dur:
			return
		case <-time.After(250 * time.Millisecond):
		}
		if canliKontrolUygula() { // v41
			return
		}
	}
	if kare == nil {
		telegramYanit(ctx, jeton, sahip, "🎥 Kameradan kare gelmedi efendim (kamera USB'de görünmüyor olabilir).")
		return
	}
	id, err := telegramFotoMesaj(ctx, jeton, sahip, kare, canliBaslik(time.Until(canliBitis())), canliDugmeler(true))
	if err != nil {
		fmt.Fprintln(os.Stderr, "canli:", err)
		return
	}
	neden, hata := "süre doldu", 0
	aralik, sonYukle, yukle, sinir429, t0 := canliAralik, time.Now(), 0, 0, time.Now() // v36
	defer func() {
		fmt.Fprintf(os.Stderr, "canli: %d düzenleme / %.0f sn (%.2f sn/kare), 429: %d, aralık %v, %s\n", yukle,
			time.Since(t0).Seconds(), time.Since(t0).Seconds()/float64(max(1, yukle)), sinir429, aralik, neden)
	}()
dongu:
	for {
		bekle := aralik - time.Since(sonYukle) // v36: aralık yüklemenin başından
		if bekle < canliEnAz {
			bekle = canliEnAz
		}
		select {
		case <-dur:
			neden = "durduruldu"
			break dongu
		case <-time.After(bekle):
		}
		if canliKontrolUygula() { // v41: başka süreçten
			neden = "durduruldu"
			break
		}
		if time.Now().After(canliBitis()) {
			break
		}
		if dosyaVar(canliKalkan) {
			neden = "gizlilik kalkanı açıldı"
			break
		}
		canliBayrakYaz(canliBitis()) // Merkez kendi bayrağını kaldırdıysa yeniden
		b, mt, ok := canliKareOku()
		if !ok || !mt.After(son) {
			continue // yeni kare yok
		}
		kare, son = b, mt
		sonYukle = time.Now()
		if e := telegramFotoDuzenle(ctx, jeton, sahip, id, kare, canliBaslik(time.Until(canliBitis())), canliDugmeler(true)); e != nil {
			if hata++; hata >= 3 {
				neden = "Telegram hatası"
				fmt.Fprintln(os.Stderr, "canli:", e)
				break
			}
			if r := tgBekleSn(e); r > 0 {
				sinir429++
				aralik = max(aralik, 2*canliAralik) // v36: Telegram yavaşla dedi: bu izlemede eski hız
				time.Sleep(time.Duration(r) * time.Second)
			}
			continue
		}
		yukle++
		hata = 0
	}
	b, _ := json.Marshal(canliDugmeler(false))
	_ = telegramIstek(ctx, jeton, "editMessageCaption", map[string][]string{"chat_id": {sahip},
		"message_id": {strconv.FormatInt(id, 10)}, "caption": {"🎥 Canlı izleme bitti (" + neden + ", " +
			time.Now().Format("15:04") + ")"}, "reply_markup": {string(b)}}, nil)
}

// tgBekleSn: "retry after N" (429) hatasından N
func tgBekleSn(e error) int {
	s := e.Error()
	if i := strings.Index(s, "retry after "); i >= 0 {
		n, _ := strconv.Atoi(strings.Fields(s[i+len("retry after "):])[0])
		if n > 0 && n <= 30 {
			return n
		}
	}
	return 0
}

// telegramFotoMesaj: sendPhoto (bayt, açıklama, satır içi düğmeler) -> message_id
func telegramFotoMesaj(ctx context.Context, jeton, sohbet string, jpg []byte, baslik string, markup any) (int64, error) {
	alan := map[string]string{"chat_id": sohbet, "caption": baslik}
	if markup != nil {
		b, _ := json.Marshal(markup)
		alan["reply_markup"] = string(b)
	}
	var m struct {
		MessageID int64 `json:"message_id"`
	}
	if err := tgCokParcali(ctx, jeton, "sendPhoto", alan, "photo", jpg, &m); err != nil {
		return 0, err
	}
	return m.MessageID, nil
}

// telegramFotoDuzenle: editMessageMedia (aynı mesajda yeni kare)
func telegramFotoDuzenle(ctx context.Context, jeton, sohbet string, mesaj int64, jpg []byte, baslik string, markup any) error {
	media, _ := json.Marshal(map[string]string{"type": "photo", "media": "attach://kare", "caption": baslik})
	alan := map[string]string{"chat_id": sohbet, "message_id": strconv.FormatInt(mesaj, 10), "media": string(media)}
	if markup != nil {
		b, _ := json.Marshal(markup)
		alan["reply_markup"] = string(b)
	}
	return tgCokParcali(ctx, jeton, "editMessageMedia", alan, "kare", jpg, nil)
}

func tgCokParcali(ctx context.Context, jeton, metod string, alan map[string]string, dosyaAd string, veri []byte, v any) error {
	var g bytes.Buffer
	w := multipart.NewWriter(&g)
	for k, d := range alan {
		_ = w.WriteField(k, d)
	}
	p, _ := w.CreateFormFile(dosyaAd, "kamera.jpg")
	_, _ = p.Write(veri)
	_ = w.Close()
	req, _ := http.NewRequestWithContext(ctx, http.MethodPost, tgTaban+"/bot"+jeton+"/"+metod, &g)
	req.Header.Set("Content-Type", w.FormDataContentType())
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<16))
	var c struct {
		Ok          bool            `json:"ok"`
		Description string          `json:"description"`
		Result      json.RawMessage `json:"result"`
	}
	if json.Unmarshal(b, &c) != nil || !c.Ok {
		return fmt.Errorf("telegram %s: HTTP %d %s", metod, res.StatusCode, c.Description)
	}
	if v != nil {
		return json.Unmarshal(c.Result, v)
	}
	return nil
}

// canliBitmesiniBekle: --telegram süreci dönerken süren izleme bitene kadar (en çok canliAzami + 30 sn) bekle
func canliBitmesiniBekle() {
	t0 := time.Now()
	for canliSurdu() && time.Since(t0) < canliAzami+30*time.Second {
		time.Sleep(500 * time.Millisecond)
	}
}

// v41 süreçler arası denetim: izlemeyi yürüten --tg-isle süreci canliPid'e yazar; sonraki --tg-isle süreçleri (⏹ / ➕
// düğmeleri, sürerken /canli) komutu canliKontrol dosyasıyla iletir, oturum her turda (≤1 sn) okur.
func canliPidYaz() {
	_ = os.Remove(canliKontrol)
	_ = os.WriteFile(canliPid, []byte(strconv.Itoa(os.Getpid())+"\n"), 0644)
}

func canliPidSil() {
	if b, err := os.ReadFile(canliPid); err == nil && strings.TrimSpace(string(b)) == strconv.Itoa(os.Getpid()) {
		_ = os.Remove(canliPid)
	}
}

// canliBaskaSurec: izleme başka (yaşayan) bir süreçte mi?
func canliBaskaSurec() bool {
	b, err := os.ReadFile(canliPid)
	if err != nil {
		return false
	}
	p, err := strconv.Atoi(strings.TrimSpace(string(b)))
	if err != nil || p <= 0 || p == os.Getpid() {
		return false
	}
	return syscall.Kill(p, 0) == nil
}

func canliKontrolYaz(k string) error {
	return dosyaYazAtomik(canliKontrol, []byte(k+"\n"), 0644) // v42: benzersiz geçici ad (iki süreç aynı anda)
}

// canliKontrolUygula: başka süreçten gelen komut (varsa) uygulanır; true = durdur
func canliKontrolUygula() bool {
	b, err := os.ReadFile(canliKontrol)
	if err != nil {
		return false
	}
	_ = os.Remove(canliKontrol)
	canliDurum.Lock()
	defer canliDurum.Unlock()
	switch strings.TrimSpace(string(b)) {
	case "dur":
		return true
	case "uzat":
		canliDurum.bitis = canliSinirla(canliDurum.bitis.Add(canliSure)) // v42
	case "yeniden":
		canliDurum.bitis = canliSinirla(time.Now().Add(canliSure))
	}
	return false
}

// canliPidTemizle (--tg-zamanli): süreci ölmüş oturumun pid ve denetim dosyaları
func canliPidTemizle() {
	if dosyaVar(canliPid) && !canliBaskaSurec() && !canliSurdu() {
		_ = os.Remove(canliPid)
		_ = os.Remove(canliKontrol)
	}
}

// canliSinirla v42: bitiş en çok oturum başı + canliAzami (canliDurum kilitliyken çağrılır)
func canliSinirla(b time.Time) time.Time {
	if lim := canliDurum.bas.Add(canliAzami); b.After(lim) {
		return lim
	}
	return b
}

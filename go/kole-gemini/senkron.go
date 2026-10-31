// senkron.go (20.10.2026, Claude): v14 ses/altyazı senkronu ve karşılama sesi; v15 kilit bekleme + hazır sesler.
package main

import (
	"context"
	"encoding/binary"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"
	"time"
)

var (
	ttsSureYol    = "/tmp/tts-sure"
	selamPCMYol   = "/data/asistan/selam.pcm"
	selamMetinYol = "/data/asistan/selam.txt"
	selamSesYol   = "/data/asistan/selam.ses"
	sesModelYol   = "/data/asistan/gemini-ses-model"
)

const (
	sesModelVarsayilan = "gemini-3.5-flash-lite,gemini-flash-lite-latest" // v31: 3.8-flash/3.1-flash-lite sık 503 (22.10)
	selamMetni         = "Buyurun efendim."
	onSessizMs         = 1000 // amfi acilisi (Codex 21:14 ilk hece teyidi)
)

// sesli soruda once tercih edilen model, sonra mevcut zincir (tekrarsiz)
func sesModelleri(tercih string, zincir []string) []string {
	o := []string{}
	gor := map[string]bool{}
	for _, m := range append(strings.Split(tercih, ","), zincir...) { // v20: tercih virgullu liste olabilir
		m = strings.TrimSpace(m)
		if m != "" && !gor[m] {
			gor[m] = true
			o = append(o, m)
		}
	}
	return o
}

// 16 kHz ornekler -> 1 sn on + 0.2 sn son sessizlikli PCM baytlari
func pcmHazirla(x []int16) []byte {
	x = sessizKirp(x) // v44: TTS'in baş/son sessizliği
	on, son := ttsHedefHz*onSessizMs/1000, ttsHedefHz/5
	b := make([]byte, 2*(on+len(x)+son))
	for i, v := range x {
		binary.LittleEndian.PutUint16(b[2*(on+i):], uint16(v))
	}
	return b
}

func sureMetni(x []int16) string {
	return fmt.Sprintf("on=%d sure=%d\n", onSessizMs, len(x)*1000/ttsHedefHz)
}

// dosyaYazAtomik v42 (denetim v2 #10): geçici dosya aynı dizinde benzersiz adla; sabit ".part" adı iki süreç (--tg-isle,
// --ses) aynı anda yazınca birbirini eziyordu (yarım dosya ya da kayıp tur)
func dosyaYazAtomik(yol string, b []byte, kip os.FileMode) error {
	eski, _ := filepath.Glob(yol + ".*.part") // kesilen yazmaların (OOM, reset) artıkları birikmesin
	for _, e := range eski {
		if st, err := os.Stat(e); err == nil && time.Since(st.ModTime()) > 10*time.Minute {
			_ = os.Remove(e)
		}
	}
	f, err := os.CreateTemp(filepath.Dir(yol), filepath.Base(yol)+".*.part")
	if err != nil {
		return err
	}
	g := f.Name()
	_, err = f.Write(b)
	if e := f.Chmod(kip); err == nil {
		err = e
	}
	if e := f.Close(); err == nil {
		err = e
	}
	if err == nil {
		err = os.Rename(g, yol)
	}
	if err != nil {
		_ = os.Remove(g)
	}
	return err
}

// --ses: v18 akışlı cevap sesi. Kilit + mikrofon bayrağı alınır, `--akis` ayrı süreçte TTS akışını köprüye çalar; ilk
// ses (ya da yedek tam ses) hazır olunca /tmp/tts-sure yazılır, JSON ondan sonra basılır (altyazı sesle başlar).
// true: ses başladı (en çok 15 sn beklenir)
func cevapSesHazirla(anahtarYol, metin string) bool {
	return cevapSesHazirlaKip(anahtarYol, metin, true)
}

// cevapSesHazirlaKip v40: ayriSurec false ise ses akışı bu süreçte goroutine (soru sunucusu; arkaIs ile beklenir)
func cevapSesHazirlaKip(anahtarYol, metin string, ayriSurec bool) bool {
	metin = strings.TrimSpace(metin)
	if !ttsAcik() || metin == "" || oku(anahtarYol, "") == "" {
		return false
	}
	gunluk := func(f string, a ...any) {
		if g, e := ttsGunlukAc(); e == nil { // v42: 64 KB sınırı
			fmt.Fprintf(g, time.Now().Format("15:04:05")+" "+f+"\n", a...)
			g.Close()
		}
	}
	if !kilitAlBekle(butce(8 * time.Second)) { // v15: asistanin hazir sesi ("dusunuyorum") bitsin; v21: butce
		gunluk("tts cevap: başka konuşma sürüyor, ses atlandı")
		return false
	}
	_ = os.Remove(ttsSureYol)
	_ = os.Remove(akisHataYol)
	aldik := mikBirakAl()
	if !ayriSurec { // v40
		arkaIs.Add(1)
		go func() {
			defer arkaIs.Done()
			defer func() {
				if r := recover(); r != nil {
					fmt.Fprintln(os.Stderr, "ses akışı panik:", r)
				}
			}()
			akisCalistirMetin(anahtarYol, metin)
		}()
		return cevapSesBekle(metin, gunluk)
	}
	exe, _ := os.Executable()
	g, _ := ttsGunlukAc() // v42
	cmd := exec.Command(exe, "--akis", "--anahtar-dosya", anahtarYol)
	cmd.Env = append(os.Environ(), "KOLE_KONUS_METIN="+metin)
	if g != nil {
		cmd.Stdout, cmd.Stderr = g, g
	}
	cmd.SysProcAttr = &syscall.SysProcAttr{Setsid: true}
	if err := cmd.Start(); err != nil {
		_ = os.Remove(ttsKilitYol)
		if aldik {
			mikBirakBirak()
		}
		if g != nil {
			g.Close()
		}
		return false
	}
	_ = cmd.Process.Release()
	if g != nil {
		g.Close()
	}
	return cevapSesBekle(metin, gunluk)
}

func cevapSesBekle(metin string, gunluk func(string, ...any)) bool {
	t0 := time.Now()
	hazir := akisHazirBekle(butce(15 * time.Second)) // v21: asistan 30 sn'de keser; JSON (metin) once basilmali
	durum := "hazır"
	if !hazir {
		durum = "HAZIR DEĞİL (süre bütçesi, metin önce; ses hazır olunca çalar)"
	}
	gunluk("tts cevap: akış %s, bekleme %d ms, %d karakter", durum, time.Since(t0).Milliseconds(), len([]rune(metin)))
	return hazir
}

// --cal: hazir PCM'i cal; kilit, mikrofon bayragi ve dosya temizlenir; sonra gerekirse karsilama sesi uretilir
func calCalistir(pcmYol, anahtarYol string) int {
	saat := func() string { return time.Now().Format("15:04:05") }
	b, err := os.ReadFile(pcmYol)
	_ = os.Remove(pcmYol)
	rc := 0
	if err != nil || len(b) == 0 {
		rc = 2
	} else if err := kopruCalBayt(b); err != nil {
		fmt.Fprintf(os.Stderr, "%s tts cal: %v\n", saat(), err)
		rc = 3
	} else {
		fmt.Fprintf(os.Stderr, "%s tts cal: %.1f sn çalındı\n", saat(), float64(len(b))/32000)
	}
	_ = os.Remove(ttsKilitYol)
	_ = os.Remove(ttsSureYol)
	mikBirakBirak()
	selamHazirla(anahtarYol)
	return rc
}

// v15: kole-asistan hazır sesi (ör. "düşünüyorum") çalarken kilit dolu; en çok azami kadar bekle
func kilitAlBekle(azami time.Duration) bool {
	son := time.Now().Add(azami)
	for {
		if kf, err := os.OpenFile(ttsKilitYol, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0644); err == nil {
			fmt.Fprintf(kf, "gemini %d\n", os.Getpid())
			kf.Close()
			return true
		}
		if !time.Now().Before(son) {
			return false
		}
		time.Sleep(100 * time.Millisecond)
	}
}

func selamSil() {
	_ = os.Remove(selamPCMYol)
	_ = os.Remove(selamMetinYol)
	_ = os.Remove(selamSesYol)
}

// karsilama sesi yoksa ya da ses degistiyse uret (cevap caldiktan sonra; gecikme yaratmaz)
func selamHazirla(anahtarYol string) {
	ses := ttsSesAdi()
	if oku(selamSesYol, "") == ses && dosyaVar(selamPCMYol) && dosyaVar(selamMetinYol) {
		return
	}
	anahtar := oku(anahtarYol, "")
	if anahtar == "" || !ttsAcik() {
		return
	}
	ctx, iptal := context.WithTimeout(context.Background(), 25*time.Second)
	defer iptal()
	x, _, err := ttsUretGemini(ctx, anahtar, selamMetni) // v17: yalniz Gemini
	if err != nil || len(x) == 0 {
		fmt.Fprintf(os.Stderr, "%s selam: üretilemedi: %v\n", time.Now().Format("15:04:05"), err)
		return
	}
	if dosyaYazAtomik(selamPCMYol, pcmHazirla(x), 0644) == nil && dosyaYazAtomik(selamMetinYol, []byte(selamMetni+"\n"), 0644) == nil {
		_ = dosyaYazAtomik(selamSesYol, []byte(ses+"\n"), 0644)
		fmt.Fprintf(os.Stderr, "%s selam: %s ile %.1f sn hazır\n", time.Now().Format("15:04:05"), ses, float64(len(x))/ttsHedefHz)
	}
}

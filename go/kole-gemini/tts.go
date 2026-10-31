// tts.go (20.10.2026, Claude): v13 SESLİ CEVAP. kullanıcı: "hey köle gerçekçi insan sesiyle konuşsun, genç; API ile".
// Gemini 3.8 Flash TTS (Interactions API) -> gerçek örnekleme hızı yanıttan (24 kHz) -> pencereli sinc FIR 16 kHz ->
// başa 1 sn / sona 0.2 sn sessizlik (amfi açılışı) -> kole-kopru PLAY (127.0.0.1:8767, anahtar /data/ses-anahtar).
// Çalarken /tmp/asistan-mik-birak ("tts" işaretli): kole-asistan mikrofonu bırakır, kendi sesine uyanmaz.
package main

import (
	"bufio"
	"bytes"
	"context"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"math"
	"net"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"syscall"
	"time"
)

// testlerde değiştirilebilsin diye değişken
var (
	ttsURL          = "https://generativelanguage.googleapis.com/v1beta/interactions"
	ttsModelYol     = "/data/asistan/tts-model"
	ttsSesYol       = "/data/asistan/tts-ses"
	ttsStilYol      = "/data/asistan/tts-stil"
	ttsKapaliYol    = "/data/asistan/tts-kapali"
	kopruAnahtarYol = "/data/ses-anahtar"
	kopruAdres      = "127.0.0.1:8767"
	mikBirakYol     = "/tmp/asistan-mik-birak"
	ttsKilitYol     = "/tmp/tts-caliyor"
	ttsGunlukYol    = "/tmp/tts.log"
)

const (
	ttsModelVarsayilan = "gemini-3.8-flash-tts"
	ttsModelYedek      = "gemini-3.8-flash-lite-tts"
	ttsAzamiSn         = 55 // köprü PLAY en çok 60 sn; ön/son sessizlik payı
	ttsHedefHz         = 16000
	ttsIsaret          = "tts"
	ttsStilVarsayilan  = "Genç, doğal ve sıcak bir Türk gibi konuş; samimi ama saygılı, akıcı ve canlı tonlamayla, " +
		"acele etmeden. Robotik ya da metin okuyor gibi değil, karşındakiyle sohbet ediyormuş gibi."
)

type ttsSes struct{ ad, tarif string }

// Gemini hazır seslerinden önerilenler (tarifler Google'ın sınıflandırması: üslup)
var ttsSesler = []ttsSes{
	{"Puck", "genç, neşeli (varsayılan)"}, {"Leda", "genç"}, {"Fenrir", "heyecanlı"}, {"Zephyr", "parlak"},
	{"Achird", "samimi"}, {"Sulafat", "sıcak"}, {"Charon", "bilgilendirici, olgun"}, {"Kore", "kararlı"},
	{"Aoede", "rahat"}, {"Algieba", "yumuşak"},
}

func ttsAcik() bool { return !dosyaVar(ttsKapaliYol) }

func ttsSesBul(s string) (string, bool) {
	s = strings.ToLower(strings.TrimSpace(s))
	for _, v := range ttsSesler {
		if strings.ToLower(v.ad) == s {
			return v.ad, true
		}
	}
	return "", false
}

func ttsSesAdi() string {
	if a, ok := ttsSesBul(oku(ttsSesYol, "")); ok {
		return a
	}
	return "Puck"
}

func ttsGovde(model, metin, ses, stil string) []byte {
	g := map[string]any{
		"model": model,
		"input": []any{map[string]any{"type": "user_input", "content": []any{map[string]any{
			"type": "text", "text": metin,
			"annotations": []any{map[string]any{"type": "speech_metadata", "style": stil}}}}}},
		"response_format":   map[string]any{"type": "audio", "mime_type": "audio/l16", "sample_rate": 24000},
		"generation_config": map[string]any{"speech_config": []any{map[string]any{"voice": ses}}},
	}
	b, _ := json.Marshal(g)
	return b
}

var oranRe = regexp.MustCompile(`rate=(\d+)`)

// yanıttaki son ses parçası -> 16 bit mono örnekler + hız
func ttsSesCoz(govde []byte) ([]int16, int, error) {
	var y struct {
		Steps []struct {
			Content []struct {
				Type       string `json:"type"`
				Data       string `json:"data"`
				MimeType   string `json:"mime_type"`
				SampleRate int    `json:"sample_rate"`
				Channels   int    `json:"channels"`
			} `json:"content"`
		} `json:"steps"`
		Error *struct {
			Message string `json:"message"`
		} `json:"error"`
	}
	if err := json.Unmarshal(govde, &y); err != nil {
		return nil, 0, fmt.Errorf("yanıt JSON değil")
	}
	if y.Error != nil {
		return nil, 0, errors.New(y.Error.Message)
	}
	var veri, mime string
	hz, kanal := 0, 1
	for _, st := range y.Steps {
		for _, c := range st.Content {
			if c.Type == "audio" && c.Data != "" {
				veri, mime, hz, kanal = c.Data, c.MimeType, c.SampleRate, c.Channels
			}
		}
	}
	if veri == "" {
		return nil, 0, fmt.Errorf("yanıtta ses yok")
	}
	ham, err := base64.StdEncoding.DecodeString(veri)
	if err != nil {
		return nil, 0, fmt.Errorf("ses base64 değil")
	}
	if strings.HasPrefix(mime, "audio/wav") || bytes.HasPrefix(ham, []byte("RIFF")) {
		return wavCoz(ham)
	}
	if hz == 0 {
		if m := oranRe.FindStringSubmatch(mime); m != nil {
			hz, _ = strconv.Atoi(m[1])
		}
	}
	if hz == 0 {
		hz = 24000 // Interactions API varsayılanı (belge + Codex ölçümü)
	}
	if kanal > 1 {
		return nil, 0, fmt.Errorf("çok kanallı ses desteklenmiyor")
	}
	if hz != 16000 && hz != 24000 && hz != 48000 {
		return nil, 0, fmt.Errorf("beklenmeyen örnekleme hızı %d", hz)
	}
	if len(ham)%2 != 0 {
		ham = ham[:len(ham)-1]
	}
	x := make([]int16, len(ham)/2)
	for i := range x {
		x[i] = int16(binary.LittleEndian.Uint16(ham[2*i:]))
	}
	return x, hz, nil
}

func wavCoz(b []byte) ([]int16, int, error) {
	if len(b) < 12 || string(b[:4]) != "RIFF" || string(b[8:12]) != "WAVE" {
		return nil, 0, fmt.Errorf("WAV değil")
	}
	hz, kanal, bit := 0, 0, 0
	for i := 12; i+8 <= len(b); {
		ad, n := string(b[i:i+4]), int(binary.LittleEndian.Uint32(b[i+4:]))
		if n < 0 || i+8+n > len(b) {
			n = len(b) - i - 8
		}
		g := b[i+8 : i+8+n]
		switch ad {
		case "fmt ":
			if len(g) >= 16 {
				kanal = int(binary.LittleEndian.Uint16(g[2:]))
				hz = int(binary.LittleEndian.Uint32(g[4:]))
				bit = int(binary.LittleEndian.Uint16(g[14:]))
			}
		case "data":
			if bit != 16 || kanal != 1 || hz == 0 {
				return nil, 0, fmt.Errorf("WAV biçimi desteklenmiyor (%d bit, %d kanal, %d Hz)", bit, kanal, hz)
			}
			x := make([]int16, len(g)/2)
			for j := range x {
				x[j] = int16(binary.LittleEndian.Uint16(g[2*j:]))
			}
			return x, hz, nil
		}
		i += 8 + n + n&1
	}
	return nil, 0, fmt.Errorf("WAV data yok")
}

// pencereli sinc (Hann, ±24 örnek) ile yeniden örnekleme; Codex dogal_ses_cihaz_prova_v4 ile aynı çekirdek
func yenidenOrnekle(x []int16, kaynak, hedef int) []int16 {
	if kaynak == hedef || len(x) == 0 {
		return append([]int16(nil), x...)
	}
	const yari = 24
	fc := 0.45 * math.Min(1, float64(hedef)/float64(kaynak))
	n := int(int64(len(x)) * int64(hedef) / int64(kaynak))
	y := make([]int16, n)
	onbellek := map[int][]float64{}
	for j := 0; j < n; j++ {
		pay := int64(j) * int64(kaynak)
		merkez := int(pay / int64(hedef))
		kalan := int(pay % int64(hedef)) // kesir = kalan/hedef
		w, ok := onbellek[kalan]
		if !ok {
			kesir := float64(kalan) / float64(hedef)
			w = make([]float64, 2*yari+1)
			top := 0.0
			for k := -yari; k <= yari; k++ {
				dd := float64(k) - kesir
				s := 2 * fc
				if math.Abs(dd) > 1e-12 {
					s = math.Sin(2*math.Pi*fc*dd) / (math.Pi * dd)
				}
				w[k+yari] = s * (0.5 + 0.5*math.Cos(math.Pi*dd/(yari+1)))
				top += w[k+yari]
			}
			for k := range w {
				w[k] /= top
			}
			onbellek[kalan] = w
		}
		v := 0.0
		for k := -yari; k <= yari; k++ {
			if i := merkez + k; i >= 0 && i < len(x) {
				v += float64(x[i]) * w[k+yari]
			}
		}
		y[j] = int16(math.Max(-32768, math.Min(32767, math.Round(v))))
	}
	return y
}

// metin -> 16 kHz örnekler. v17: sağlayıcı rotasyonu ve soğuma (saglayici.go)
func ttsUret(ctx context.Context, anahtar, metin string) ([]int16, string, error) {
	return ttsUretSec(ctx, anahtar, metin, false)
}

// köprüye PLAY: "<anahtar> PLAY <bayt>\n" + PCM, yanıt "OK 0"
func kopruCal(x []int16) error {
	return kopruCalBayt(pcmHazirla(x)) // v14: 1 sn ön, 0.2 sn kuyruk (senkron.go)
}

func kopruCalBayt(pcm []byte) error {
	anahtar := oku(kopruAnahtarYol, "")
	if anahtar == "" {
		return fmt.Errorf("köprü anahtarı yok")
	}
	c, err := net.DialTimeout("tcp", kopruAdres, 3*time.Second)
	if err != nil {
		return err
	}
	defer c.Close()
	_ = c.SetDeadline(time.Now().Add(time.Duration(len(pcm)/32000+30) * time.Second))
	if _, err := fmt.Fprintf(c, "%s PLAY %d\n", anahtar, len(pcm)); err != nil {
		return err
	}
	if _, err := c.Write(pcm); err != nil {
		return err
	}
	sat, _ := bufio.NewReader(c).ReadString('\n')
	sat = strings.TrimSpace(sat)
	if sat != "OK 0" {
		return fmt.Errorf("köprü yanıtı %q", sat)
	}
	return nil
}

// mikrofon bayrağı: yalnız bizim yazdığımızı ("tts") sileriz (kole-kopru uzaktan dinleme de aynı dosyayı kullanır)
func mikBirakAl() bool {
	f, err := os.OpenFile(mikBirakYol, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0644)
	if err != nil {
		return false
	}
	fmt.Fprintf(f, "%s %d\n", ttsIsaret, os.Getpid())
	f.Close()
	return true
}

func mikBirakBirak() {
	if b, err := os.ReadFile(mikBirakYol); err == nil && strings.HasPrefix(string(b), ttsIsaret+" ") {
		_ = os.Remove(mikBirakYol)
	}
}

func mikBirakTemizle() {
	st, err := os.Stat(mikBirakYol)
	if err == nil && time.Since(st.ModTime()) > 120*time.Second {
		mikBirakBirak()
	}
	if st, err := os.Stat(ttsKilitYol); err == nil && time.Since(st.ModTime()) > 120*time.Second {
		_ = os.Remove(ttsKilitYol)
	}
}

// ttsGunlukAc v42 (denetim v2 #8): /tmp/tts.log (RAM) 64 KB'ı aşınca sıfırlanır; bütün yazarlar bunu kullanır
func ttsGunlukAc() (*os.File, error) {
	if st, err := os.Stat(ttsGunlukYol); err == nil && st.Size() > 64<<10 {
		_ = os.Truncate(ttsGunlukYol, 0)
	}
	return os.OpenFile(ttsGunlukYol, os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0644)
}

// konuşmayı ayrı süreçte başlat (çağıran hemen döner)
func konusArkaPlan(metin string) bool {
	metin = strings.TrimSpace(metin)
	if !ttsAcik() || metin == "" {
		return false
	}
	exe, err := os.Executable()
	if err != nil {
		return false
	}
	g, _ := ttsGunlukAc()
	cmd := exec.Command(exe, "--konus") // v42: mikrofon bayrağını --konus kilidi aldıktan sonra kendisi alır
	cmd.Env = append(os.Environ(), "KOLE_KONUS_METIN="+metin)
	if g != nil {
		cmd.Stdout, cmd.Stderr = g, g
	}
	cmd.SysProcAttr = &syscall.SysProcAttr{Setsid: true}
	if err := cmd.Start(); err != nil {
		if g != nil {
			g.Close()
		}
		return false
	}
	_ = cmd.Process.Release()
	if g != nil {
		g.Close()
	}
	return true
}

// --konus: tek konuşma (kilit), TTS + köprü; bitince mikrofon bayrağı silinir
func konusCalistir(anahtarYol string) int {
	metin := strings.TrimSpace(os.Getenv("KOLE_KONUS_METIN"))
	saat := func() string { return time.Now().Format("15:04:05") }
	if metin == "" {
		return 2
	}
	kf, err := os.OpenFile(ttsKilitYol, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0644)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s tts: başka konuşma sürüyor, atlandı\n", saat())
		return 3
	}
	kf.Close()
	defer os.Remove(ttsKilitYol)
	if mikBirakAl() { // v42 (denetim v2 #5): bayrak kilitten sonra alınır, kilitten önce bırakılır (--ses ile aynı sıra)
		defer mikBirakBirak()
	}
	anahtar := oku(anahtarYol, "")
	if anahtar == "" {
		fmt.Fprintf(os.Stderr, "%s tts: Gemini anahtarı yok\n", saat())
		return 4
	}
	ctx, iptal := context.WithTimeout(context.Background(), 60*time.Second)
	defer iptal()
	t0 := time.Now()
	x, ses, err := ttsUret(ctx, anahtar, metin)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s tts: üretilemedi (%s): %v\n", saat(), ses, err)
		return 5
	}
	uretim := time.Since(t0)
	if err := kopruCal(x); err != nil {
		fmt.Fprintf(os.Stderr, "%s tts: çalınamadı: %v\n", saat(), err)
		return 6
	}
	fmt.Fprintf(os.Stderr, "%s tts: %s, %.1f sn ses, üretim %d ms, toplam %d ms, %d karakter\n", saat(), ses,
		float64(len(x))/ttsHedefHz, uretim.Milliseconds(), time.Since(t0).Milliseconds(), len([]rune(metin)))
	return 0
}

// Telegram komutları
func sesKomutu(metin string) string {
	if metin == "" {
		return "Kullanım: /ses merhaba efendim"
	}
	if !ttsAcik() {
		return "Sesli konuşma kapalı efendim. Açmak için /sesac."
	}
	if !konusArkaPlan(metin) {
		return "Şu an söyleyemiyorum efendim."
	}
	return "🔊 Söylüyorum efendim (ses: " + ttsSesAdi() + ")."
}

func seslerKomutu(arg string) string {
	if arg != "" {
		ad, ok := ttsSesBul(arg)
		if !ok {
			return "Bu sesi tanımıyorum efendim. /sesler ile listeye bakın."
		}
		if err := os.WriteFile(ttsSesYol, []byte(ad+"\n"), 0644); err != nil {
			return "Ses kaydedilemedi efendim."
		}
		selamSil() // v14: karsilama yeni sesle yeniden uretilsin
		sogumaKaldir("tts:gcloud")
		hazirSil() // v15: hazir sesler de (sonraki --veri / cevapta yeniden)
		konusArkaPlan("Merhaba efendim, yeni sesim bu. Nasıl buldunuz?")
		return "🔊 Ses " + ad + " oldu efendim, örnek söylüyorum."
	}
	var b strings.Builder
	b.WriteString("🔊 Sesler (şu an: " + ttsSesAdi() + ")\n")
	for _, s := range ttsSesler {
		b.WriteString("• " + s.ad + " — " + s.tarif + "\n")
	}
	b.WriteString("Değiştirmek için: /sesler Leda")
	return b.String()
}

func sesAcKapa(ac bool) string {
	if ac {
		_ = os.Remove(ttsKapaliYol)
		return "🔊 Hey Köle cevaplarını artık sesli söylüyorum efendim."
	}
	_ = os.WriteFile(ttsKapaliYol, []byte("1\n"), 0644)
	return "🔇 Hey Köle cevaplarını yalnız ekranda gösteririm efendim."
}

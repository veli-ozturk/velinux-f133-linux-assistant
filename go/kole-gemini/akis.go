// akis.go v26 (23.10.2026, Claude; kole-gemini v42, scripts/598): ilk ses parçasından sonra sessizlik bekçisi (akisBekci,
// akisSessizSure; takılan akış Hey Köle'yi 90 sn sağır bırakıyordu), akışta da 700 karakter sınırı (ttsKirp). --- v25 ---
// akis.go v25 (23.10.2026, Claude; kole-gemini v39, scripts/562): akisCal, Gemini TTS modellerinin hepsi soğumadaysa ve
// soğumada olmayan bir yedek sağlayıcı varsa akışı atlar (akisAtlaNedeni); tam yol doğrudan yedekle başlar.
// akis.go v24 (21.10.2026, Claude; scripts/439): köprü üretim hızına göre ön tamponla açılır (ontampon.go), altyazı
// köprü açılınca. --- v18 ---
// akis.go (20.10.2026, Claude): v18 AKIŞLI TTS (scripts/381). Hey Köle cevabı Gemini TTS akışıyla (Interactions API
// "stream": true) parça parça çalınır: ilk ses parçasında köprüye PLAY_AKIS (kole-kopru v8+), 1 sn sıfır (amfi; köprü
// v9'da ön sessizlik yok; kole-kopru v10 kurulursa onSessizMs 0 olmalı) + 16 kHz parçalar. Altyazı (panel 0.49+)
// /tmp/tts-sure ilk parçada yazılınca başlar. Akış ses vermezse v17 tam yolu (ttsUretSec rotasyonu + PLAY).
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
	"io"
	"math"
	"net"
	"net/http"
	"os"
	"strconv"
	"strings"
	"sync/atomic"
	"time"
)

var akisHataYol = "/tmp/tts-akis-hata"

var akisSessizSure = 5 * time.Second // v42: ilk sesten sonra ağdan bu kadar veri gelmezse akış kesilir (gelen kadarı çalınır)

const (
	akisHarfMs  = 69   // Puck ölçümü (20.10): 108 harf 7.5 sn
	akisCerceve = 8192 // bayt; köprü PLAY_AKIS çerçeve sınırı
	akisFazSure = 8 * time.Second
)

// ---------------------------------------------------------------- parça parça yeniden örnekleme

// yenidenOrnekle ile AYNI pencereli sinc; çıkış örneği tüm dokuları (merkez±24) geldiyse üretilir, son'da kalanlar
type akisOrnekleyici struct {
	kaynak, hedef int
	x             []int16
	j             int
	onbellek      map[int][]float64
}

func yeniAkisOrnekleyici(kaynak, hedef int) *akisOrnekleyici {
	return &akisOrnekleyici{kaynak: kaynak, hedef: hedef, onbellek: map[int][]float64{}}
}

func (a *akisOrnekleyici) ekle(p []int16, son bool) []int16 {
	a.x = append(a.x, p...)
	if a.kaynak == a.hedef || a.kaynak <= 0 {
		y := append([]int16(nil), a.x[a.j:]...)
		a.j = len(a.x)
		return y
	}
	const yari = 24
	fc := 0.45 * math.Min(1, float64(a.hedef)/float64(a.kaynak))
	n := int(int64(len(a.x)) * int64(a.hedef) / int64(a.kaynak))
	var y []int16
	for ; a.j < n; a.j++ {
		pay := int64(a.j) * int64(a.kaynak)
		merkez := int(pay / int64(a.hedef))
		if !son && merkez+yari >= len(a.x) {
			break
		}
		kalan := int(pay % int64(a.hedef))
		w, ok := a.onbellek[kalan]
		if !ok {
			kesir := float64(kalan) / float64(a.hedef)
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
			a.onbellek[kalan] = w
		}
		v := 0.0
		for k := -yari; k <= yari; k++ {
			if i := merkez + k; i >= 0 && i < len(a.x) {
				v += float64(a.x[i]) * w[k+yari]
			}
		}
		y = append(y, int16(math.Max(-32768, math.Min(32767, math.Round(v)))))
	}
	return y
}

// ---------------------------------------------------------------- akış ayrıştırıcı

type akisSes struct {
	x  []int16
	hz int
}

// v içinde type=="audio" ve data dizgesi olan nesneler (her derinlikte)
func akisSesBul(v any, varsayilanHz int) []akisSes {
	var o []akisSes
	var gez func(any)
	gez = func(x any) {
		switch t := x.(type) {
		case map[string]any:
			if tip, _ := t["type"].(string); tip == "audio" {
				if veri, _ := t["data"].(string); veri != "" {
					if ham, err := base64.StdEncoding.DecodeString(veri); err == nil {
						mime, _ := t["mime_type"].(string)
						hz := varsayilanHz
						if f, ok := t["sample_rate"].(float64); ok && f > 0 {
							hz = int(f)
						} else if m := oranRe.FindStringSubmatch(mime); m != nil {
							hz, _ = strconv.Atoi(m[1])
						}
						if strings.HasPrefix(mime, "audio/wav") || bytes.HasPrefix(ham, []byte("RIFF")) {
							if x, whz, err := wavCoz(ham); err == nil {
								o = append(o, akisSes{x, whz})
							}
						} else {
							o = append(o, akisSes{pcmBayt16(ham), hz})
						}
					}
					return
				}
			}
			for _, d := range t {
				gez(d)
			}
		case []any:
			for _, d := range t {
				gez(d)
			}
		}
	}
	gez(v)
	return o
}

// Interactions API akışı: SSE ("data: {...}"), satır başına JSON ya da (çok satırlı) JSON dizi. Yalnız DELTA ses
// parçaları f'ye gider; delta hiç gelmezse son tam ses en sonda verilir. Dönüş: f çağrı sayısı.
func akisSesParcalari(r io.Reader, f func(x []int16, hz int) error) (int, error) {
	br := bufio.NewReaderSize(r, 64<<10)
	var sse, birikim bytes.Buffer
	sayi, sonHz := 0, 24000
	var tam []akisSes
	isle := func(b []byte) (bool, error) { // true: JSON nesnesi çözüldü
		b = bytes.TrimSpace(b)
		b = bytes.TrimSpace(bytes.TrimLeft(b, "[,"))
		b = bytes.TrimSpace(bytes.TrimRight(b, "],"))
		if len(b) == 0 || b[0] != '{' {
			return true, nil // boş ya da JSON değil ("[DONE]" vb.): atla
		}
		var v map[string]any
		if json.Unmarshal(b, &v) != nil {
			return false, nil
		}
		if e, ok := v["error"].(map[string]any); ok {
			msg, _ := e["message"].(string)
			return true, fmt.Errorf("akış hatası: %s", msg)
		}
		et, _ := v["event_type"].(string)
		delta, deltaVar := v["delta"]
		if deltaVar || strings.Contains(et, "delta") {
			kaynak := any(v)
			if deltaVar {
				kaynak = delta
			}
			for _, s := range akisSesBul(kaynak, sonHz) {
				sonHz = s.hz
				sayi++
				if err := f(s.x, s.hz); err != nil {
					return true, err
				}
			}
			return true, nil
		}
		if s := akisSesBul(v, sonHz); len(s) > 0 {
			tam = s
		}
		return true, nil
	}
	for {
		satir, err := br.ReadBytes('\n')
		if len(satir) > 0 {
			t := bytes.TrimRight(satir, "\r\n")
			switch {
			case bytes.HasPrefix(t, []byte("data:")):
				if sse.Len() > 0 {
					sse.WriteByte('\n')
				}
				sse.Write(bytes.TrimSpace(t[5:]))
			case len(bytes.TrimSpace(t)) == 0:
				if sse.Len() > 0 {
					if _, e := isle(sse.Bytes()); e != nil {
						return sayi, e
					}
					sse.Reset()
				}
			case bytes.HasPrefix(t, []byte("event:")) || bytes.HasPrefix(t, []byte("id:")) || bytes.HasPrefix(t, []byte(":")) ||
				bytes.HasPrefix(t, []byte("retry:")):
			default: // satır başına JSON ya da çok satırlı dizi: çözülene kadar biriktir
				birikim.Write(t)
				birikim.WriteByte('\n')
				ok, e := isle(birikim.Bytes())
				if e != nil {
					return sayi, e
				}
				if ok || birikim.Len() > 8<<20 {
					birikim.Reset()
				}
			}
		}
		if err != nil {
			if sse.Len() > 0 {
				if _, e := isle(sse.Bytes()); e != nil {
					return sayi, e
				}
			}
			if err != io.EOF {
				return sayi, err
			}
			break
		}
	}
	if sayi == 0 {
		for _, s := range tam {
			sayi++
			if err := f(s.x, s.hz); err != nil {
				return sayi, err
			}
		}
	}
	return sayi, nil
}

// ---------------------------------------------------------------- köprü PLAY_AKIS istemcisi

type akisKopru struct {
	c          net.Conn
	br         *bufio.Reader
	gonderilen int // örnek
}

// "<anahtar> PLAY_AKIS <sn>\n" -> "PLAY_AKIS HAZIR 16000\n"
func akisKopruAc(sn int) (*akisKopru, error) {
	anahtar := oku(kopruAnahtarYol, "")
	if anahtar == "" {
		return nil, fmt.Errorf("köprü anahtarı yok")
	}
	c, err := net.DialTimeout("tcp", kopruAdres, 3*time.Second)
	if err != nil {
		return nil, err
	}
	_ = c.SetDeadline(time.Now().Add(5 * time.Second))
	if _, err := fmt.Fprintf(c, "%s PLAY_AKIS %d\n", anahtar, sn); err != nil {
		c.Close()
		return nil, err
	}
	br := bufio.NewReader(c)
	sat, _ := br.ReadString('\n')
	if sat = strings.TrimSpace(sat); sat != "PLAY_AKIS HAZIR 16000" {
		c.Close()
		return nil, fmt.Errorf("köprü yanıtı %q", sat)
	}
	_ = c.SetDeadline(time.Time{})
	return &akisKopru{c: c, br: br}, nil
}

func (k *akisKopru) yaz(x []int16) error {
	for len(x) > 0 {
		n := len(x)
		if n > akisCerceve/2 {
			n = akisCerceve / 2
		}
		b := make([]byte, 2+2*n)
		binary.LittleEndian.PutUint16(b, uint16(2*n))
		for i, v := range x[:n] {
			binary.LittleEndian.PutUint16(b[2+2*i:], uint16(v))
		}
		_ = k.c.SetWriteDeadline(time.Now().Add(10 * time.Second))
		if _, err := k.c.Write(b); err != nil {
			return err
		}
		k.gonderilen += n
		x = x[n:]
	}
	return nil
}

// n=0 son çerçevesi; köprü çalmayı bitirince "OK <ornek> <xrun>"
func (k *akisKopru) bitir() (int, int, error) {
	defer k.c.Close()
	_ = k.c.SetWriteDeadline(time.Now().Add(10 * time.Second))
	if _, err := k.c.Write([]byte{0, 0}); err != nil {
		return 0, 0, err
	}
	_ = k.c.SetReadDeadline(time.Now().Add(time.Duration(k.gonderilen/16+10000) * time.Millisecond))
	sat, err := k.br.ReadString('\n')
	if err != nil && sat == "" {
		return 0, 0, err
	}
	f := strings.Fields(sat)
	if len(f) != 3 || f[0] != "OK" {
		return 0, 0, fmt.Errorf("köprü yanıtı %q", strings.TrimSpace(sat))
	}
	o, _ := strconv.Atoi(f[1])
	x, _ := strconv.Atoi(f[2])
	return o, x, nil
}

// ---------------------------------------------------------------- akış oturumu

type akisSonuc struct {
	model                           string
	ilkMs                           int64
	ornek, xrun                     int
	basladi, biriktirdi, yarimKaldi bool // basladi: ilk ses parçası geldi (ilkMs yerelde 0 olabilir)
}

var errAkisSesYok = errors.New("akış ses vermedi")

func akisGovde(model, metin, ses, stil string) []byte {
	var g map[string]any
	_ = json.Unmarshal(ttsGeminiGovde(model, metin, ses, stil), &g)
	g["stream"] = true
	b, _ := json.Marshal(g)
	return b
}

// tek model: istek -> parçalar köprüye. sinir: ilk ses parçası için kalan süre
func akisModel(ctx context.Context, anahtar, model, metin, ses, stil string, sinir time.Duration, t0 time.Time) (akisSonuc, error) {
	sonuc := akisSonuc{model: model}
	stilli := stil != ""
	if _, ok := sogumaDurum("tts-stilsiz:" + model); ok {
		stilli = false
	}
	for {
		st := stil
		if !stilli {
			st = ""
		}
		rctx, iptal := context.WithCancel(ctx)
		var ilk atomic.Bool
		bekci := &akisBekci{ilk: &ilk}
		zam := time.AfterFunc(sinir, func() { // ilk ses için sinir; v42: sonra sessizlik bekçisi (akisBekci kurar)
			if !ilk.Load() || bekci.okuyor.Load() {
				iptal()
			}
		})
		bekci.zam = zam
		req, _ := http.NewRequestWithContext(rctx, http.MethodPost, ttsURL, bytes.NewReader(akisGovde(model, metin, ses, st)))
		req.Header.Set("Content-Type", "application/json")
		req.Header.Set("Accept", "text/event-stream, application/json")
		req.Header.Set("x-goog-api-key", anahtar)
		res, err := http.DefaultClient.Do(req)
		if err != nil {
			zam.Stop()
			iptal()
			return sonuc, errors.New(gizle(err.Error(), anahtar))
		}
		if res.StatusCode != 200 {
			b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<20))
			res.Body.Close()
			zam.Stop()
			iptal()
			temiz := gizle(string(b), anahtar)
			msg := temiz
			if len(msg) > 160 {
				msg = msg[:160]
			}
			ad := "tts:gemini:" + model
			switch res.StatusCode {
			case 429:
				kotaKaydet(model, []byte(temiz), res.Header.Get("Retry-After"))
				d := sogumaKoyKota(ad, []byte(temiz), res.Header.Get("Retry-After"))
				hazirGunluk("tts akış: %s 429, %d dk sona alındı", model, int(d.Minutes()+0.5))
			case 401, 403, 404:
				sogumaKoy(ad, 24*time.Hour, strconv.Itoa(res.StatusCode))
			case 400:
				if stilli && stilHataRe.MatchString(temiz) {
					stilli = false
					sogumaKoy("tts-stilsiz:"+model, 7*24*time.Hour, "stil")
					continue
				}
				sogumaKoy(ad, 6*time.Hour, "400")
			}
			return sonuc, fmt.Errorf("%s: HTTP %d %s", model, res.StatusCode, msg)
		}
		bekci.r = res.Body
		r, err := akisOturum(bekci, &ilk, metin, t0, &sonuc)
		res.Body.Close()
		zam.Stop()
		iptal()
		return r, err
	}
}

// akisBekci v42 (denetim v2 #11): ilk ses parçasından sonra ağdan her okuma en çok akisSessizSure bekler. Köprüye
// yazarken (geri basınç) geçen süre sayılmaz: sayaç yalnız okuma sürerken kurulu.
type akisBekci struct {
	r      io.Reader
	ilk    *atomic.Bool
	zam    *time.Timer
	okuyor atomic.Bool
}

func (b *akisBekci) Read(p []byte) (int, error) {
	if !b.ilk.Load() {
		return b.r.Read(p)
	}
	b.okuyor.Store(true)
	b.zam.Reset(akisSessizSure)
	n, err := b.r.Read(p)
	b.okuyor.Store(false)
	b.zam.Stop()
	return n, err
}

// akış gövdesi -> köprü (ya da köprü reddederse biriktir + PLAY)
func akisOturum(govde io.Reader, ilk *atomic.Bool, metin string, t0 time.Time, sonuc *akisSonuc) (akisSonuc, error) {
	var orn *akisOrnekleyici
	var kp *akisKopru
	var birikim []int16
	var kopruHata error
	var bekleyen []int16 // v24: köprü açılana kadar (ön tampon)
	var ilkT time.Time
	ilkParcaMs := 0
	tahmin := len([]rune(metin)) * akisHarfMs
	kopruAc := func(neden string, hiz float64) error { // v24: köprü + altyazı + amfi sessizliği + biriken
		k, e := akisKopruAc(60)
		if e != nil {
			hazirGunluk("tts akış: köprü PLAY_AKIS olmadı (%v), biriktirip tam çalınacak", e)
			sonuc.biriktirdi = true
			birikim = append(birikim, bekleyen...)
			bekleyen = nil
			return nil
		}
		kp = k
		hazirGunluk("tts akış: ön tampon %d ms (%s, hız %.2f, tahmin %d ms)", len(bekleyen)*1000/ttsHedefHz, neden, hiz, tahmin)
		_ = dosyaYazAtomik(ttsSureYol, []byte(fmt.Sprintf("on=%d sure=%d\n", onSessizMs, tahmin)), 0644)
		if e := kp.yaz(make([]int16, ttsHedefHz*onSessizMs/1000)); e != nil {
			kopruHata = e
			return e
		}
		b := bekleyen
		bekleyen = nil
		if e := kp.yaz(b); e != nil {
			kopruHata = e
			return e
		}
		return nil
	}
	cikis := func(y []int16) error {
		sonuc.ornek += len(y)
		if kp != nil {
			if err := kp.yaz(y); err != nil {
				kopruHata = err
				return err
			}
			return nil
		}
		if sonuc.biriktirdi {
			birikim = append(birikim, y...)
			return nil
		}
		bekleyen = append(bekleyen, y...) // v24: köprü henüz açılmadı
		return nil
	}
	_, err := akisSesParcalari(govde, func(x []int16, hz int) error {
		if orn == nil {
			orn = yeniAkisOrnekleyici(hz, ttsHedefHz)
		}
		y := orn.ekle(x, false)
		ilkMi := !ilk.Load()
		if ilkMi {
			ilk.Store(true)
			sonuc.basladi = true
			sonuc.ilkMs = time.Since(t0).Milliseconds()
			ilkT = time.Now()
			ilkParcaMs = len(y) * 1000 / ttsHedefHz
		}
		if e := cikis(y); e != nil {
			return e
		}
		if kp == nil && !sonuc.biriktirdi { // v24: yeterli ön tampon birikti mi
			if ac, hiz := onTamponYeter(len(bekleyen)*1000/ttsHedefHz, ilkParcaMs, time.Since(ilkT), tahmin); ac {
				return kopruAc("yeterli", hiz)
			}
		}
		return nil
	})
	if orn == nil { // hiç ses yok
		if err == nil {
			err = errAkisSesYok
		}
		return *sonuc, err
	}
	if err != nil && kopruHata == nil {
		sonuc.yarimKaldi = true // akış yarıda kesildi: gelen kadarı çalınır
		hazirGunluk("tts akış: yarıda kesildi (%v)", err)
	}
	if kopruHata == nil {
		_ = cikis(orn.ekle(nil, true))
		if kp == nil && !sonuc.biriktirdi && len(bekleyen) > 0 { // v24: akış bitti, köprü hiç açılmadı
			_ = kopruAc("akış bitti", 0)
		}
	}
	if kp != nil {
		if kopruHata != nil {
			kp.c.Close()
			return *sonuc, fmt.Errorf("köprü: %v", kopruHata)
		}
		_, xr, e := kp.bitir()
		sonuc.xrun = xr
		if e != nil {
			return *sonuc, fmt.Errorf("köprü bitiş: %v", e)
		}
		return *sonuc, nil
	}
	_ = dosyaYazAtomik(ttsSureYol, []byte(sureMetni(birikim)), 0644)
	if e := kopruCalBayt(pcmHazirla(birikim)); e != nil {
		return *sonuc, fmt.Errorf("köprü PLAY: %v", e)
	}
	return *sonuc, nil
}

// akisAtlaNedeni v39: Gemini TTS modellerinin hepsi soğumadaysa ve soğumada olmayan Gemini dışı bir sağlayıcı varsa
// (ElevenLabs/Azure/Google Cloud) o sağlayıcının adı; yoksa "" (akış denenir). Kota doluyken akış fazı 8 sn'ye kadar yiyordu.
func akisAtlaNedeni(anahtar string) string {
	for _, m := range ttsGeminiListesi() {
		if _, ok := sogumaDurum("tts:gemini:" + m); !ok {
			return ""
		}
	}
	for _, a := range ttsAdaylar(anahtar, "", ttsSesAdi(), "", false) {
		if strings.HasPrefix(a.ad, "gemini:") {
			continue
		}
		if _, ok := sogumaDurum("tts:" + a.ad); !ok {
			return a.ad
		}
	}
	return ""
}

// Gemini adayları soğuma sırasıyla; ilk ses için hepsine toplam akisFazSure
func akisCal(ctx context.Context, anahtar, metin string, t0 time.Time) (akisSonuc, error) {
	if y := akisAtlaNedeni(anahtar); y != "" { // v39
		hazirGunluk("tts akış: Gemini TTS hepsi soğumada, akış atlandı (yedek: %s)", y)
		return akisSonuc{}, errors.New("Gemini TTS hepsi soğumada, yedek " + y)
	}
	ses := ttsSesAdi()
	stil := oku(ttsStilYol, ttsStilVarsayilan)
	son := errors.New("akış adayı yok")
	bitis := time.Now().Add(akisFazSure)
	for _, m := range sogumaSirala(ttsGeminiListesi(), "tts:gemini:") {
		kalan := time.Until(bitis)
		if kalan <= 0 {
			break
		}
		r, err := akisModel(ctx, anahtar, m, metin, ses, stil, kalan, t0)
		if r.basladi { // ses başladı: başka modele geçilmez (çift ses olmasın)
			if err == nil {
				sogumaKaldir("tts:gemini:" + m)
			}
			return r, err
		}
		son = err
	}
	return akisSonuc{}, son
}

// --akis: alt süreç. Kilit, tts-sure ve mikrofon bayrağı sonda temizlenir
func akisCalistir(anahtarYol string) int {
	return akisCalistirMetin(anahtarYol, strings.TrimSpace(os.Getenv("KOLE_KONUS_METIN")))
}

// akisCalistirMetin v40: soru sunucusunda goroutine olarak da çalışır
func akisCalistirMetin(anahtarYol, metin string) int {
	temiz := false
	temizle := func() { // karşılama sesi üretilmeden önce de çağrılır (kilit gereksiz tutulmasın)
		if !temiz {
			temiz = true
			_ = os.Remove(ttsSureYol)
			_ = os.Remove(ttsKilitYol)
			mikBirakBirak()
		}
	}
	defer temizle()
	hata := func() { _ = os.WriteFile(akisHataYol, []byte("1\n"), 0644) }
	metin = ttsKirp(strings.TrimSpace(metin)) // v42 (denetim v2 #17): tam yoldaki 700 karakter sınırı akışta da
	anahtar := oku(anahtarYol, "")
	if metin == "" || anahtar == "" {
		hata()
		return 2
	}
	t0 := time.Now()
	ctx, iptal := context.WithTimeout(context.Background(), 90*time.Second)
	defer iptal()
	r, err := akisCal(ctx, anahtar, metin, t0)
	if r.basladi {
		ek := ""
		if r.biriktirdi {
			ek = ", köprü akışsız (biriktirildi)"
		}
		if r.yarimKaldi {
			ek += ", akış yarıda kesildi"
		}
		if err != nil {
			ek += ", hata: " + gizle(err.Error(), anahtar)
		}
		hazirGunluk("tts akış: %s, ilk ses %d ms, %.1f sn, xrun %d%s", r.model, r.ilkMs, float64(r.ornek)/ttsHedefHz, r.xrun, ek)
		temizle()
		selamHazirla(anahtarYol)
		if err != nil {
			return 3
		}
		return 0
	}
	hazirGunluk("tts akış: ses yok (%v), tam yola geçiliyor", err)
	x, ses, err := ttsUretSec(ctx, anahtar, metin, false)
	if err != nil || len(x) == 0 {
		hazirGunluk("tts akış: yedek de olmadı (%s): %v", ses, err)
		hata()
		return 5
	}
	_ = dosyaYazAtomik(ttsSureYol, []byte(sureMetni(x)), 0644)
	if err := kopruCalBayt(pcmHazirla(x)); err != nil {
		hazirGunluk("tts akış: yedek çalınamadı: %v", err)
		return 6
	}
	hazirGunluk("tts akış: yedek tam yol %s, %.1f sn, toplam %d ms", ses, float64(len(x))/ttsHedefHz, time.Since(t0).Milliseconds())
	temizle()
	selamHazirla(anahtarYol)
	return 0
}

// --ses ana süreci: /tmp/tts-sure (ses başladı) ya da /tmp/tts-akis-hata görünene kadar
func akisHazirBekle(azami time.Duration) bool {
	son := time.Now().Add(azami)
	for time.Now().Before(son) {
		if dosyaVar(ttsSureYol) {
			return true
		}
		if dosyaVar(akisHataYol) {
			return false
		}
		time.Sleep(50 * time.Millisecond)
	}
	return false
}

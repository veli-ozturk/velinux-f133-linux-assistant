// kota.go (20.10.2026, Claude): v16 TTS KOTA TANILAMASI (scripts/363). 20.10'da Gemini TTS 429 verdi, sınırın türü
// (dakika/gün) bilinmiyordu. 429 yanıtından kota adı/değeri ve bekleme süresi (RetryInfo.retryDelay ya da Retry-After)
// çıkarılır, /tmp/tts-kota ve /tmp/tts.log'a yazılır (anahtarsız). Hazır ses üretimi bekleme bitene kadar istek atmaz.
package main

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

var ttsKotaYol = "/tmp/tts-kota"

type ttsKota struct {
	Zaman int64  `json:"zaman"`
	Model string `json:"model"`
	Kota  string `json:"kota,omitempty"`
	Deger string `json:"deger,omitempty"`
	Bekle int    `json:"bekle_sn,omitempty"`
	Mesaj string `json:"mesaj,omitempty"`
}

// Google RPC hata ayrıntıları: anahtarlar her derinlikte aranır (QuotaFailure.violations, RetryInfo)
func kotaCoz(govde []byte, retryAfter string) (kota, deger string, bekle int, mesaj string) {
	var v any
	var id, metrik string
	if json.Unmarshal(govde, &v) == nil {
		var gez func(any)
		gez = func(x any) {
			switch t := x.(type) {
			case map[string]any:
				for k, d := range t {
					s, _ := d.(string)
					switch k {
					case "quotaId":
						id = s
					case "quotaMetric":
						metrik = s
					case "quotaValue":
						if s == "" {
							if f, ok := d.(float64); ok {
								s = strconv.FormatFloat(f, 'f', -1, 64)
							}
						}
						deger = s
					case "retryDelay":
						if n, err := time.ParseDuration(s); err == nil {
							bekle = int(n.Seconds() + 0.5)
						}
					}
					gez(d)
				}
			case []any:
				for _, d := range t {
					gez(d)
				}
			}
		}
		gez(v)
		var e struct {
			Error struct {
				Message string `json:"message"`
			} `json:"error"`
		}
		if json.Unmarshal(govde, &e) == nil {
			mesaj = e.Error.Message
		}
	}
	kota = id
	if kota == "" {
		kota = metrik
	}
	if bekle == 0 {
		if n, err := strconv.Atoi(strings.TrimSpace(retryAfter)); err == nil && n > 0 {
			bekle = n
		}
	}
	if r := []rune(mesaj); len(r) > 120 {
		mesaj = string(r[:120])
	}
	return
}

func bosIse(s, v string) string {
	if s == "" {
		return v
	}
	return s
}

func kotaKaydet(model string, govde []byte, retryAfter string) {
	k := ttsKota{Zaman: time.Now().Unix(), Model: model}
	k.Kota, k.Deger, k.Bekle, k.Mesaj = kotaCoz(govde, retryAfter)
	if b, err := json.Marshal(k); err == nil {
		_ = dosyaYazAtomik(ttsKotaYol, append(b, '\n'), 0644)
	}
	hazirGunluk("tts kota: %s 429, kota %s (%s), bekle %d sn", model, bosIse(k.Kota, "?"), bosIse(k.Deger, "?"), k.Bekle)
}

func kotaOku() (ttsKota, bool) {
	var k ttsKota
	b, err := os.ReadFile(ttsKotaYol)
	if err != nil || json.Unmarshal(b, &k) != nil || k.Zaman == 0 {
		return k, false
	}
	return k, true
}

// son 429'un bekleme süresi sürüyor mu (kalan sn)
func kotaBekliyor() (bool, int) {
	k, ok := kotaOku()
	if !ok || k.Bekle <= 0 {
		return false, 0
	}
	kalan := k.Zaman + int64(k.Bekle) - time.Now().Unix()
	return kalan > 0, int(kalan)
}

// Telegram /sesdurum
func sesDurumu() string {
	var b strings.Builder
	ac := "açık"
	if !ttsAcik() {
		ac = "KAPALI (/sesac ile açılır)"
	}
	fmt.Fprintf(&b, "🔊 Ses durumu\nSes: %s · Hey Köle sesli cevap %s\n", ttsSesAdi(), ac)
	if dosyaVar(hazirKok) {
		fmt.Fprintf(&b, "Hazır cümleler: %d/%d\n", len(hazirCumleler)-hazirEksik(), len(hazirCumleler))
	} else {
		b.WriteString("Hazır cümleler: SD kart bağlı değil\n")
	}
	if k, ok := kotaOku(); ok {
		fmt.Fprintf(&b, "Son kota hatası: %s, %s", time.Unix(k.Zaman, 0).In(trZaman).Format("02.01 15:04:05"), k.Model)
		if k.Kota != "" {
			fmt.Fprintf(&b, ", kota %s", k.Kota)
			if k.Deger != "" {
				fmt.Fprintf(&b, " = %s", k.Deger)
			}
		}
		if k.Bekle > 0 {
			fmt.Fprintf(&b, ", bekleme %d sn", k.Bekle)
		}
		b.WriteString("\n")
	} else {
		b.WriteString("Kota hatası yok (bu açılıştan beri).\n")
	}
	b.WriteString(saglayiciDurumu()) // v17
	// v23: AssemblyAI konuşma -> metin yedeği
	if dosyaVar(assemblyAnahtarYol) {
		b.WriteString("\nKonuşma yedeği (AssemblyAI): kayıtlı")
	} else {
		b.WriteString("\nKonuşma yedeği (AssemblyAI): yok (/anahtar assembly <anahtar>)")
	}
	return strings.TrimSpace(b.String())
}

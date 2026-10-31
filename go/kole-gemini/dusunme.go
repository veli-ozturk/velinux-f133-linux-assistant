// dusunme.go (21.10.2026, Claude; kole-gemini v26, OTONOM Sıra 8 madde 57): Hey Köle düşünme seviyesi Telegram'dan
// seçilir ve cevap süreleri görülür.
// Ölçüm (madde 57, docs/sira8-olcumler-v1.md): sesli istek ~1.5-2 bin belirteç (sistem talimatı ~1000, ses 32/sn, en çok 4
// tur geçmiş), gövde en çok 0.42 MB (10 sn ses). Gecikmeyi boyut değil model süresi belirliyor (20.10 cihaz günlükleri:
// 3.8-flash 7.9-14.1 sn, 3.1-flash-lite 8.0-13.7 sn). Varsayılan "low" DEĞİŞMEDİ; /dusunme hizli (minimal) denenip
// /hiz ile (asistan.log "model X, N ms (dusunme S)" satırları) kıyaslanır. Model thinkingConfig'i reddederse yaris.go
// ayarsız gövdeyle yeniden dener (v2'den beri).
package main

import (
	"fmt"
	"os"
	"strings"
)

var (
	dusunmeYol    = "/data/asistan/dusunme"
	asistanGunluk = "/tmp/asistan.log"
)

var dusunmeAdlari = []struct{ ad, seviye, aciklama string }{
	{"hizli", "minimal", "en az düşünme, en kısa bekleme"},
	{"dengeli", "low", "varsayılan"},
	{"dikkatli", "medium", "daha iyi anlama, daha uzun bekleme"},
}

func dusunmeSeviyesi() string {
	v := oku(dusunmeYol, "low")
	for _, d := range dusunmeAdlari {
		if v == d.seviye {
			return v
		}
	}
	return "low"
}

// dusunmeKomutu: "/dusunme" (durum), "/dusunme hizli|dengeli|dikkatli" (ya da minimal|low|medium)
func dusunmeKomutu(arg string) string {
	arg = strings.ToLower(strings.TrimSpace(arg))
	if arg == "" {
		simdi := dusunmeSeviyesi()
		s := "🧠 Hey Köle düşünme seviyesi: "
		for _, d := range dusunmeAdlari {
			if d.seviye == simdi {
				s += d.ad + " (" + d.aciklama + ")"
			}
		}
		s += "\n\n"
		for _, d := range dusunmeAdlari {
			s += fmt.Sprintf("/dusunme %s — %s\n", d.ad, d.aciklama)
		}
		return s + "/hiz — son cevap süreleri (kıyas için)"
	}
	for _, d := range dusunmeAdlari {
		if arg != d.ad && arg != d.seviye {
			continue
		}
		var err error
		if d.seviye == "low" {
			if err = os.Remove(dusunmeYol); os.IsNotExist(err) {
				err = nil
			}
		} else {
			err = os.WriteFile(dusunmeYol, []byte(d.seviye+"\n"), 0644)
		}
		if err != nil {
			return "Kaydedemedim efendim: " + err.Error()
		}
		return fmt.Sprintf("Tamam efendim, düşünme seviyesi: %s (%s). Sonraki sorudan itibaren geçerli; /hiz ile süreleri "+
			"kıyaslayabilirsiniz.", d.ad, d.aciklama)
	}
	return "Bu seviyeyi bilmiyorum efendim. /dusunme yazın, seçenekleri göstereyim."
}

// hizKomutu: asistan.log'daki son 8 cevap süresi ("model X, N ms" satırları; v26'dan önce düşünme yazılmazdı)
func hizKomutu() string {
	b, err := os.ReadFile(asistanGunluk)
	if err != nil {
		return "Asistan günlüğü yok efendim (cihaz yeni açılmış olabilir)."
	}
	var son []string
	for _, s := range strings.Split(string(b), "\n") {
		if strings.HasPrefix(s, "model ") && strings.Contains(s, " ms") {
			son = append(son, strings.TrimSpace(strings.TrimPrefix(s, "model ")))
		}
	}
	if len(son) == 0 {
		return "Henüz Hey Köle cevabı yok efendim. Bir soru sorun, süreyi burada gösteririm."
	}
	if len(son) > 8 {
		son = son[len(son)-8:]
	}
	return "⏱ Son Hey Köle cevap süreleri (model, süre, düşünme):\n" + strings.Join(son, "\n") +
		"\n\nŞu anki seviye: " + dusunmeSeviyesi() + " — değiştirmek için /dusunme"
}

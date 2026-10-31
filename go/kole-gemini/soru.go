// soru.go (23.10.2026, Claude; kole-gemini v40, madde 131): v39 main'deki --ses/--metin gövdesi AYNEN (os.Exit(N) ->
// return N). Hem tek seferlik süreç (main) hem sürekli --telegram süreci (soru_sunucu.go) bunu çağırır.
package main

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"os"
	"strings"
	"time"
)

type soruIstek struct {
	Ses   string `json:"ses,omitempty"`   // 16 kHz mono WAV yolu (okunur, silinir)
	Metin string `json:"metin,omitempty"` // metin soru
}

type soruYollar struct {
	anahtar, model, yuz, gecmis, sehir, eylem string
	sure                                      time.Duration
	ayriSurec                                 bool // ses akışı ayrı süreçte (tek seferlik main) ya da goroutine (sunucu)
}

// soruIsle: rc 0 ise s geçerli; değilse v39'daki çıkış kodu (3 anahtar yok, 4 ses okunamadı, 5 zaman/ağ, 6 cevap yok)
func soruIsle(yol soruYollar, q soruIstek) (sonuc, int) {
	surecBas = time.Now() // v40: butce soru başından (uzun yaşayan süreçte süreç başından sayılırsa 0 döner)
	anahtar := oku(yol.anahtar, "")
	if anahtar == "" {
		fmt.Fprintln(os.Stderr, "anahtar yok:", yol.anahtar)
		return sonuc{}, 3
	}
	modeller := strings.Split(oku(yol.model, varsayilanModel), ",")
	metinModelleri := append([]string{}, modeller...) // v23: AssemblyAI yedeği metin modellerine sorar
	if q.Ses == "" {
		cikisSiniri = metinCikisSiniri // v21: panel --metin 45 sn bekler
	} else {
		cikisSiniri = sesliCikisSiniri // v40: aynı süreçte sırayla sesli/metin
	}
	if q.Ses != "" { // v14: konusmayi daha iyi anlayan model once
		modeller = sesModelleri(oku(sesModelYol, sesModelVarsayilan), modeller)
	}
	k := karakterOku(yol.yuz)
	simdi := time.Now().Format("2 January 2006 Monday 15:04")
	g := gecmisOku(yol.gecmis)

	icerikler := gecmisIcerik(g) // v27: boş soru/cevaplı tur isteğe girmez (HTTP 400)
	var son []parca
	var sesWav []byte // v23: AssemblyAI yedeği için (yalnız bellekte; dosya hemen silinir)
	if q.Ses != "" {
		wav, err := os.ReadFile(q.Ses)
		_ = os.Remove(q.Ses) // ses RAM'de bile tutulmaz
		if err != nil || len(wav) < 1000 {
			fmt.Fprintln(os.Stderr, "ses okunamadi")
			return sonuc{}, 4
		}
		sesWav = wav
		son = []parca{{InlineData: &inlineData{MimeType: "audio/wav", Data: base64.StdEncoding.EncodeToString(wav)}},
			{Text: "Bu kayıt sahibinin sana sesli sorusu ya da isteği. 'soru' alanına anladığın soruyu Türkçe yaz, 'cevap' alanına ona cevabını yaz. Kayıt gürültülü ya da " +
				"kısık olabilir; sözcükleri dikkatle çöz. Soruyu gerçekten anlayamıyorsan ya da kayıt boşsa TAHMİN ETME: 'soru' " +
				"alanını boş bırak, 'cevap' alanına 'Tam anlayamadım efendim, tekrar eder misiniz?' yaz."}}
	} else {
		son = []parca{{Text: q.Metin + "\n('soru' alanına bu soruyu, 'cevap' alanına cevabını yaz.)"}}
	}
	icerikler = append(icerikler, icerik{Role: "user", Parts: son})
	haberBaglam := panoHaberBaglam(time.Now()) // v33
	ist := istek{
		SystemInstruction: icerik{Parts: []parca{{Text: karakterTalimat[k] + " " + fmt.Sprintf(ortakTalimat, simdi) + eylemTalimat + notlarBaglam() + haberBaglam}}}, // v32; v33 bir kez
		Contents:          icerikler,
		GenerationConfig: map[string]any{
			"temperature":      0.7,
			"maxOutputTokens":  2048,
			"responseMimeType": "application/json",
			"responseSchema": map[string]any{
				"type": "OBJECT",
				"properties": map[string]any{
					"soru":  map[string]any{"type": "STRING"},
					"cevap": map[string]any{"type": "STRING"},
					"eylemler": map[string]any{
						"type": "ARRAY",
						"items": map[string]any{
							"type": "OBJECT",
							"properties": map[string]any{
								"ad":    map[string]any{"type": "STRING", "enum": eylemEnum(haberBaglam != "")}, // v33
								"deger": map[string]any{"type": "STRING"},
							},
							"required": []string{"ad", "deger"},
						},
					},
				},
				"required": []string{"soru", "cevap"},
			},
			"thinkingConfig": map[string]any{"thinkingLevel": dusunmeSeviyesi()}, // v26: /dusunme
		},
	}
	ctx, iptal := context.WithTimeout(context.Background(), yol.sure)
	defer iptal()
	bas := time.Now()
	modeller = sogumaSirala(modeller, "llm:") // v17: kotasi dolanlar sona
	yedek := llmMetinYedek
	if q.Ses != "" {
		yedek = llmSesliYedek
	}
	y, model, sonucKod := llmYaris(ctx, modeller, ist, anahtar, yedek, 2) // v20: yavas modelin yanina paralel yedek
	assemblySoru := ""
	if q.Ses != "" && (sonucKod == yarisYok || sonucKod == yarisAg) { // v23: sesli modeller düştü -> AssemblyAI + metin
		if y2, m2, metin2, ok := assemblyYedek(ctx, sesWav, ist, metinModelleri, anahtar); ok {
			y, model, sonucKod, assemblySoru = y2, m2, yarisTamam, metin2
		}
	}
	switch sonucKod {
	case yarisZaman:
		fmt.Fprintln(os.Stderr, "zaman asimi")
		return sonuc{}, 5
	case yarisAg:
		return sonuc{}, 5
	case yarisYok:
		fmt.Fprintln(os.Stderr, "hicbir model cevap vermedi")
		return sonuc{}, 6
	}
	var metinCevap strings.Builder
	for _, p := range y.Candidates[0].Content.Parts {
		metinCevap.WriteString(p.Text)
	}
	var s sonuc
	if json.Unmarshal([]byte(metinCevap.String()), &s) != nil || strings.TrimSpace(s.Cevap) == "" {
		ham := strings.TrimSpace(metinCevap.String())
		if strings.HasPrefix(ham, "{") { // v42 (denetim v2 #18): yarım/bozuk JSON seslendirilmez, geçmişe yazılmaz (hazır hata sesi)
			fmt.Fprintln(os.Stderr, "gecersiz JSON cevap")
			return sonuc{}, 6
		}
		s = sonuc{Soru: q.Metin, Cevap: ham}
	}
	s.Cevap = strings.Join(strings.Fields(s.Cevap), " ")
	if assemblySoru != "" && strings.TrimSpace(s.Soru) == "" { // v23
		s.Soru = assemblySoru
	}
	if s.Cevap == "" {
		fmt.Fprintln(os.Stderr, "bos cevap")
		return sonuc{}, 6
	}
	for _, e := range s.Eylemler { // bilgi islemleri: gercek veri, cevap yerelde kurulur (uydurma yok)
		var govde string
		var err error
		bctx, bi := context.WithTimeout(context.Background(), butce(12*time.Second)) // v21: cikis sinirini asmasin
		switch e.Ad {
		case "hava":
			govde, err = havaCumlesi(bctx, e.Deger, yol.sehir)
		case "doviz":
			govde, err = dovizCumlesi(bctx, e.Deger)
		case "namaz":
			govde, err = namazCumlesi(bctx, e.Deger, yol.sehir)
		case "haber":
			if strings.Contains(trKucuk(e.Deger), "siber") { // v32
				govde, err = siberCumlesi(bctx)
			} else {
				govde, err = haberCumlesi(bctx)
			}
		case "hatirlatici_listesi":
			govde = hatirlaticiListesi()
		default:
			bi()
			continue
		}
		bi()
		if err != nil {
			fmt.Fprintln(os.Stderr, "bilgi:", e.Ad, err)
			s.Cevap = karakterCumle(k, "Şu an bu bilgiye ulaşamadım")
		} else {
			s.Cevap = karakterCumle(k, govde)
		}
		break
	}
	n := eylemleriYaz(s.Eylemler, yol.eylem)
	fmt.Fprintf(os.Stderr, "eylem: %d onerildi, %d uygulandi\n", len(s.Eylemler), n)
	g = gecmisOku(yol.gecmis) // v42 (denetim v2 #10): ayrı süreçte aynı anda süren soru bu arada yazmış olabilir
	gecmiseEkle(&g, s)        // v27: anlaşılmayan tur yazılmaz
	gecmisYaz(yol.gecmis, g)
	fmt.Fprintf(os.Stderr, "model %s, %d ms (dusunme %s)\n", model, time.Since(bas).Milliseconds(), dusunmeSeviyesi())
	if q.Ses != "" { // v13: Hey Köle cevabını sesli söyle (ayrı süreç; altyazı beklemez)
		mikBirakTemizle()
		cevapSesHazirlaKip(yol.anahtar, s.Cevap, yol.ayriSurec) // v18: akis ilk sesi hazir olunca JSON; v40: sunucuda goroutine
	}
	return s, 0
}

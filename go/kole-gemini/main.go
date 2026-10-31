// kole-gemini v51 (31.10.2026, Claude; scripts/681): v50 + düz Telegram cevapları kalıcı menüyle (Gezi istemi menüyü gizliyordu).
// kole-gemini v50 (31.10.2026, Claude; scripts/680): v49 + 🗾 Gezi düğmesi klavyenin en üstünde (gezi dönüşü geri alınacak).
// kole-gemini v49 (31.10.2026, Claude; scripts/679): v48 (jp düğmesi, sayfa) + /jp hatırlatma durumları + 15 dk önbellek.
// kole-gemini v47 (30.10.2026, Claude; scripts/677): v46 + /jp cevabının başında "🤖 Gemini · site bilgisi".
// kole-gemini v46 (30.10.2026, Claude; scripts/676): v45 + Telegram /jp <soru>: gezi bilgi paketi (jp.go).
// kole-gemini v45 (30.10.2026, Claude; scripts/674): v44 + Telegram izinli sohbetler (izinli.go): eşi de komut verir,
// cevap yazana; sabah özeti ve uyarılar sahip + izinlilere.
// kole-gemini v42 (23.10.2026, Claude; scripts/598): v41 + denetim v2 düzeltmeleri (docs/denetim-2026-10-23-v2.md):
// #5 --konus kilit doluyken başkasının mikrofon bayrağını silmez; #8 tts.log tek yardımcıyla 64 KB; #10 paylaşılan
// dosyalar benzersiz geçici adla (dosyaYazAtomik), soru geçmişi yazmadan önce yeniden okunur; #11 akış sessizlik
// bekçisi (5 sn) + tam yolda sağlayıcı başına 10 sn; #17 akışta 700 karakter; #18 bozuk JSON cevap rc 6; #20 yedek
// bildirim yolu kötü dosyayı .bozuk yapıp sürer; R2 sahip yalnız yerel izin + özel sohbetten; /canli uzatma sınırı
// oturum başından 5 dk.
// kole-gemini v41 (23.10.2026, Claude; scripts/586): v40 + Telegram tek atış kipleri --tg-isle / --tg-zamanli (tg_tek.go;
// sürekli döngü C kole-telgraf-v1'de), canlı izleme süreçler arası denetim (canli_tg.go v3).
// kole-gemini v40 (23.10.2026, Claude; scripts/564): v39 + sorular sürekli --telegram sürecinde (soru.go, soru_sunucu.go):
// /tmp/kole-gemini.sock; --ses/--metin ince istemci; Telegram serbest metni süreç açmadan; sunucuda ses akışı goroutine.
// kole-gemini v39 (23.10.2026, Claude; scripts/562): v38 + Gemini TTS hepsi soğumadayken (ve yedek varken) akış atlanır,
// doğrudan yedek (ElevenLabs) — kota doluyken cevap sesi 13.7 sn yerine birkaç saniyede.
// kole-gemini v38 (23.10.2026, Claude; scripts/560): v37 + ElevenLabs varsayılan sesi premade Liam (Josh kütüphane sesi,
// ücretsizde 402), 402 soğuma + günlük, diğer sağlayıcı hataları da günlükte (Gemini TTS kotası dolunca ses yoktu).
// kole-gemini v37 (23.10.2026, Claude; scripts/555): v36 + OOM bildiriminde kapanan süreç ve tetikleyen (oom_kmsg.go,
// /dev/kmsg; kendini yeniden başlatan kole-gemini ise ℹ️ "işlem gerekmez").
// kole-gemini v36 (23.10.2026, Claude; scripts/553): v35 + Telegram canlı izleme saniyede bir (aralık yüklemenin başından;
// 429 gelirse o izlemede 2 sn), izleme sonu ölçüm satırı.
// kole-gemini v35 (23.10.2026, Claude; scripts/551): v34 + Hey Köle aç/kapat (heykole.go): /heykole, 🔊 Ses menüsü,
// /durum satırı, sesle kapatma (panel 0.75).
// kole-gemini v34 (22.10.2026, Claude; scripts/547): v33 + time-lapse kip seçimi (zaman_kip.go): /zaman aralık 1..3600 sn ve
// olay tespiti, ⏱ düğmesinde durum + seçim düğmeleri, sesle aralık/olay (panel 0.74).
// kole-gemini v33 (22.10.2026, Claude; scripts/538): v32 + haber önbelleği tazeyken "haber" eylemi şemadan çıkar (model
// başlıkları Türkçe özetler; v32 siber soruda İngilizce başlıkları okutuyordu).
// kole-gemini v32 (22.10.2026, Claude; scripts/536): v31 + sesle panel sesi (eylem "ses", panel 0.73) + haber önbelleği
// (/tmp/haber-onbellek.json: Pano turu yazar; haber/siber haber soruları bağlamdan, ek indirmesiz).
// kole-gemini v31 (22.10.2026, Claude; scripts/531): v30 + Hey Köle "bağlantı kuramıyorum" / kaçamak cevaplar: model
// sırası 3.5-flash-lite önde (2.5-flash-lite 404 çıktı), 503 soğuması 15 dk, sesli yedek 7 sn, istem genel bilgiyi
// doğrudan yanıtlar.
// kole-gemini v30 (22.10.2026, Claude; scripts/523): v29 + Telegram içinde canlı kamera (canli_tg.go: /canli -> tek
// fotoğraf mesajı ~2 sn'de bir editMessageMedia; ⏹ Durdur, ➕ 60 sn, en çok 5 dk; gizlilik kalkanına uyar).
// kole-gemini v29 (21.10.2026, Claude; scripts/490): v28 + siber güvenlik haberleri (haber_siber.go: bülten kategorisi,
// /haber siber, menü düğmesi, Pano'da son 2 başlık).
// kole-gemini v28 (21.10.2026, Claude; scripts/476): v27 + Pano verisi sürekli --telegram sürecinde (veri.go; autorun'un
// ayrı --veri süreci Telegram yaşarken ağa çıkmaz: 10 dk'da bir ~12 MB'lık süreç yok).
// kole-gemini v27 (21.10.2026, Claude; scripts/462): v26 + boş soru geçmiş hatası (anlaşılamayan soru sonrası 10 dk her
// istek HTTP 400 -> "bağlantı kuramıyorum"); gecmis.go.
// kole-gemini v26 (21.10.2026, Claude; scripts/449): v25 + Telegram /dusunme (Hey Köle düşünme seviyesi) ve /hiz (son
// cevap süreleri); günlükte "(dusunme S)". Varsayılan low aynı.
// kole-gemini v25 (21.10.2026, Claude; scripts/447): v24 + --telegram aynı süreçte 10 dk tur tekrarı (saatte ~64 yerine
// ~6 süreç başlatma ve TLS el sıkışma; düğmeler 15 sn uyku boşluğunu beklemez).
// kole-gemini v24 (21.10.2026, Claude; scripts/439): v23 + akışlı TTS ön tamponu (ontampon.go; xrun 13 -> 0 hedefi).
// kole-gemini v23 (21.10.2026, Claude; scripts/437): v22 + AssemblyAI konuşma -> metin yedeği (assembly.go): sesli modeller
// düşerse ses metne çevrilip metin modellerine sorulur; /anahtar assembly.
// kole-gemini v22 (21.10.2026, Claude; scripts/428): v21 + Telegram tek dokunuş menü (kalıcı düğmeler, satır içi
// düğmeler, sol menü setMyCommands), bot profil fotoğrafı = KÖLE logosu (setMyProfilePhoto), uzun sorgu.
// kole-gemini v21 (20.10.2026, Claude): v20 + ses yetismezse metin (butce.go: sesli 27 sn cikis siniri, TTS ve
// bilgi beklemeleri butceye bagli), sesli yedek gemini-3.1-flash-lite, 2.5-flash cikarildi (404) (scripts/396).
// kole-gemini v20 (20.10.2026, Claude): v19 + yavas modelin yanina paralel yedek istek (sesli 9 sn, metin 8 sn),
// deneme sureleri gunlukte, sesli yedek gemini-2.5-flash (yaris.go, scripts/394).
// kole-gemini v19 (20.10.2026, Claude): v18 + model basina sure (sesli 11 sn, metin 14 sn; son model
// kalan sure) ve 503/5xx/yavas model 3 dk soguma (model_sure.go, scripts/392).
// kole-gemini v18 (20.10.2026, Claude): v17 + akisli TTS: Hey Kole cevabi ilk ses parcasinda kopru PLAY_AKIS ile
// calar (akis.go, --akis), JSON ilk ses hazir olunca basilir; akis olmazsa v17 tam yolu (scripts/381).
// kole-gemini v17 (20.10.2026, Claude): v16 + ucretsiz saglayici rotasyonu (saglayici.go: 4 Gemini TTS modeli, Google
// Cloud / Azure / ElevenLabs anahtar varsa), 429 soguma hafizasi (TTS + metin), Telegram /anahtar (scripts/379).
// kole-gemini v16 (20.10.2026, Claude): v15 + TTS kota tanilamasi (/tmp/tts-kota, kota.go), hazir ses uretimi kota
// beklemesine uyar, Telegram /sesdurum (scripts/363).
// kole-gemini v15 (20.10.2026, Claude): v14 + hazir sesler: Hey Kole sabit cumleleri (dusunuyorum, hatalar,
// karsilamalar) bir kez TTS ile SD'ye (hazir.go; --veri ve --hazir-sesler, calistirma basina en cok 8), cevap sesi
// kilidi bekler (scripts/357); TTS 429/503'te yedek model (scripts/362).
// kole-gemini v14 (20.10.2026, Claude): v13 + ses/altyazi senkronu (ses JSON'dan once; /tmp/tts-sure), karsilama sesi
// (/data/asistan/selam.pcm), sesli soruda gemini-3.8-flash once + "anlamadim" talimati (scripts/349).
// kole-gemini v13 (20.10.2026, Claude): v12 + Hey Köle sesli cevap (Gemini TTS, tts.go; --konus) ve Telegram /ses,
// /sesler, /seskapat, /sesac (scripts/347).
// kole-gemini v12 (19.10.2026, Claude): v11 + 7 gunluk hava, panel-veri "gunler" + 1900 bayt siniri (scripts/323).
// kole-gemini v11 (19.10.2026, Claude): v10 + sayac.go (bellek/OOM/Wi-Fi hata sayaclari, /tmp/saglik.log, esik uyarilari; scripts/296).
// kole-gemini v10 (19.10.2026, Claude): v9 + haber.go (kategorili haber, /haber), ayrıntılı Telegram yardımı, sabah özetinde ezan yok.
// kole-gemini v9 (15.10.2026, Claude): v8 + saglik.go (Telegram /saglik, /sabah, sabah ozeti, hizmet uyarilari; scripts/262).
// kole-gemini v8 (15.10.2026, Claude): v7 + Telegram fotoğrafı -> dijital çerçeve (cerceve.go), /cerceve, ekran_koruyucu cerceve (scripts/238).
// kole-gemini v7 (15.10.2026, Claude): v6 + telegram.go (iki yonlu Telegram), kamera islemleri (scripts/223).
// kole-gemini v6 (15.10.2026, Claude): v5 + ek.go/foto.go (hatirlatici, alarm, namaz, haber, notlar, Telegram), TR saati.
// kole-gemini v5 (15.10.2026, Claude): v4 + isik=parlaklik, goreli parlaklik (scripts/210).
// kole-gemini v4 (15.10.2026, Claude): v3 + bilgi.go (hava/doviz PC'siz, --veri Pano verisi), muzik islemi,
// ev sehri Ankara (/data/asistan/sehir). scripts/199.
// kole-gemini v3 (15.10.2026, Claude): v2 + panel islemleri (eylem.go): yanitta "eylemler", dogrulanip
// /tmp/asistan-eylem.json (panel 0.32). kullanıcı: "panelde bir sey sordugumda diger ozellikleri yapmasini ekleyelim".
// kole-gemini v2 (15.10.2026, Claude): v1 + model zinciri (429/503/404/5xx -> sonraki model), thinkingLevel "low"
// (kabul edilmezse ayarsiz yeniden), maxOutputTokens 2048 (v1 400: dusunme belirtecleri JSON'u yarida kesiyordu).
// Canli olcum 15.10: flash-latest ve 3.8-flash sik 503/429, flash-lite-latest 3.1 sn 200.
// --- v1 ---
// kole-gemini v1 (15.10.2026, Claude): VeLinux KÖLE sesli asistanının Gemini istemcisi (cihazda, PC'siz).
// kullanıcı: "LLM lazımsa Gemini API var ... benimle hep efendi şeklinde konuşsun, o bir köle olduğunu bilsin".
//
//	kole-gemini --ses /tmp/asistan-soru.wav   (16 kHz mono WAV; kole-asistan "Hey Köle"den sonra kaydeder)
//	kole-gemini --metin "saat kaç"             (deneme)
//
// Çıktı (stdout, tek satır JSON): {"soru":"<anlaşılan soru>","cevap":"<söylenecek cevap>"}; hata: çıkış kodu != 0.
// Anahtar /data/asistan/gemini-anahtar (0600; ASLA günlüğe/çıktıya yazılmaz), model /data/asistan/gemini-model
// (yoksa gemini-flash-latest), karakter /data/asistan/yuz.conf "karakter=". Kısa bağlam: /tmp/asistan-gecmis.json
// (yalnız metin, son 4 soru-cevap, 10 dk hareketsizlikte unutulur). Ses dosyası okunduktan sonra SİLİNİR.
// HTTPS kökleri: SSL_CERT_FILE (cihazda /mnt/sd/velinux/tailscale/ca-certificates.crt).
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

const (
	varsayilanModel = "gemini-3.5-flash-lite,gemini-flash-lite-latest,gemini-3.8-flash,gemini-3.1-flash-lite,gemini-flash-latest" // v31
	gecmisN         = 4
	gecmisOmur      = 10 * time.Minute
)

var karakterTalimat = []string{
	"Sen VeLinux KÖLE'sin: sahibinin masaüstü panelinde yaşayan, ona bütün kalbiyle bağlı sadık bir kölesin. " +
		"Sahibine HER cümlede ya da en azından her cevapta 'efendim' diye hitap edersin, onun kölesi olduğunu bilir ve bundan gurur duyarsın. " +
		"Saygılı, alçakgönüllü ve hizmete hevesli konuşursun.",
	"Sen VeLinux KÖLE'sin: sahibinin hizmetindeki resmî ve ağırbaşlı bir uşaksın, aynı zamanda onun kölesi olduğunu bilirsin. " +
		"Sahibine daima 'efendim' diye hitap edersin; kibar, ölçülü ve resmî bir dil kullanırsın ('buyurunuz', 'arz ederim' gibi).",
	"Sen VeLinux KÖLE'sin: sahibinin neşeli, şakacı küçük robot kölesisin. Sahibine her zaman 'efendim' diye hitap edersin " +
		"ve onun kölesi olduğunu bilirsin. Enerjik ve sevimli konuşursun, ara sıra 'bip bop' gibi robot sesleri katarsın.",
}

const ortakTalimat = "Kurallar: Yalnız Türkçe konuş. Cevabın sesli okunacak ve ekranda altyazı olarak görünecek: " +
	"en fazla 2-3 kısa cümle, konuşma dili; markdown, liste, emoji, kod, bağlantı KULLANMA. " +
	"Genel bilgi sorularını (mesafe, yolculuk süresi, tarih, tanım, hesap, tarif gibi) bildiğin kadarıyla, gerekirse " +
	"'yaklaşık' diyerek doğrudan yanıtla; 'bilmiyorum' deyip geçme. Öneri ya da fikir istenirse somut bir öneri ver, " +
	"geri soru sorma. Yalnız o ana ait canlı bir veri gerekiyorsa (anlık fiyat, canlı skor, bir uçuşun şu anki durumu gibi) " +
	"ve bu veri sana verilmemişse bunu kısaca söyle, yine de bildiğin genel bilgiyi ekle; canlı veriyi uydurma. " + // v31
	"Ses kaydı anlaşılmıyorsa ya da boşsa kibarca tekrar etmesini iste. " +
	"Şu anki tarih ve saat: %s. "

type parca struct {
	Text       string      `json:"text,omitempty"`
	InlineData *inlineData `json:"inline_data,omitempty"`
}
type inlineData struct {
	MimeType string `json:"mime_type"`
	Data     string `json:"data"`
}
type icerik struct {
	Role  string  `json:"role,omitempty"`
	Parts []parca `json:"parts"`
}
type istek struct {
	SystemInstruction icerik         `json:"system_instruction"`
	Contents          []icerik       `json:"contents"`
	GenerationConfig  map[string]any `json:"generationConfig"`
}
type yanit struct {
	Candidates []struct {
		Content icerik `json:"content"`
	} `json:"candidates"`
	Error *struct {
		Code    int    `json:"code"`
		Message string `json:"message"`
	} `json:"error"`
}
type sonuc struct {
	Soru     string  `json:"soru"`
	Cevap    string  `json:"cevap"`
	Eylemler []eylem `json:"eylemler,omitempty"`
}
type gecmis struct {
	Zaman int64   `json:"zaman"`
	Tur   []sonuc `json:"tur"`
}

func oku(yol, varsayilan string) string {
	b, err := os.ReadFile(yol)
	if err != nil {
		return varsayilan
	}
	s := strings.TrimSpace(string(b))
	if s == "" {
		return varsayilan
	}
	return s
}

func karakterOku(yol string) int {
	b, err := os.ReadFile(yol)
	if err != nil {
		return 0
	}
	for _, sat := range strings.Split(string(b), "\n") {
		if v, ok := strings.CutPrefix(strings.TrimSpace(sat), "karakter="); ok {
			if n, err := strconv.Atoi(v); err == nil && n >= 0 && n < len(karakterTalimat) {
				return n
			}
		}
	}
	return 0
}

func gecmisOku(yol string) gecmis {
	var g gecmis
	if b, err := os.ReadFile(yol); err == nil {
		_ = json.Unmarshal(b, &g)
	}
	if time.Since(time.Unix(g.Zaman, 0)) > gecmisOmur {
		g = gecmis{}
	}
	return g
}

func gecmisYaz(yol string, g gecmis) {
	if len(g.Tur) > gecmisN {
		g.Tur = g.Tur[len(g.Tur)-gecmisN:]
	}
	g.Zaman = time.Now().Unix()
	b, _ := json.Marshal(g)
	_ = dosyaYazAtomik(yol, b, 0600) // v42: benzersiz geçici ad (Telegram ve Hey Köle soruları ayrı süreçte aynı anda)
}

func main() {
	time.Local = trZaman // cihazda zoneinfo yok
	ses := flag.String("ses", "", "16 kHz mono WAV soru kaydi (okunduktan sonra silinir)")
	metin := flag.String("metin", "", "metin soru (deneme)")
	anahtarYol := flag.String("anahtar-dosya", "/data/asistan/gemini-anahtar", "API anahtari dosyasi")
	modelYol := flag.String("model-dosya", "/data/asistan/gemini-model", "model adi dosyasi")
	yuzYol := flag.String("yuz", "/data/asistan/yuz.conf", "karakter ayari")
	gecmisYol := flag.String("gecmis", "/tmp/asistan-gecmis.json", "kisa baglam (yalniz metin)")
	saglikYaz := flag.Bool("saglik-yaz", false, "deneme: sabah ozeti + saglik stdout (Telegram yok)")
	telegramKip := flag.Bool("telegram", false, "Telegram: kuyrugu gonder + sahibin mesajlarini isle (v25: 10 dk tekrar; v28: Pano verisi de), sonra cik")
	bildirimKip := flag.Bool("bildirim-gonder", false, "/tmp/bildirim-kuyruk -> Telegram, sonra cik")
	veriKip := flag.Bool("veri", false, "Pano verisi (hava+doviz) yaz ve cik; Gemini cagrilmaz")
	veriYol := flag.String("veri-dosya", "/tmp/panel-veri.json", "Pano verisi dosyasi")
	sehirYol := flag.String("sehir-dosya", "/data/asistan/sehir", "ev sehri: Ad;enlem;boylam")
	eylemYol := flag.String("eylem", "/tmp/asistan-eylem.json", "panel islemleri (panel 0.32 okur)")
	akisKip := flag.Bool("akis", false, "(ic, v18) KOLE_KONUS_METIN metnini akisli TTS ile kopruden cal ve cik")
	calDosya := flag.String("cal", "", "(ic, v14) hazir PCM dosyasini kopruden cal, sil ve cik")
	hazirKip := flag.Bool("hazir-sesler", false, "(v15) Hey Kole sabit cumlelerinin eksik seslerini SD'ye uret ve cik")
	konusKip := flag.Bool("konus", false, "(ic, v13) KOLE_KONUS_METIN metnini Gemini TTS ile hoparlorden soyle ve cik")
	sure := flag.Duration("zaman-asimi", 25*time.Second, "istek zaman asimi")
	tgIsle := flag.String("tg-isle", "", "(v41) kole-telgraf'in verdigi tek Telegram guncellemesini (JSON dosyasi) isle ve cik")
	tgZamanli := flag.Bool("tg-zamanli", false, "(v41) Telegram zamanli islerini (kurulum, logo, sabah, uyarilar) bir kez yap ve cik")
	flag.Parse()
	if *konusKip {
		os.Exit(konusCalistir(*anahtarYol))
	}
	if *akisKip {
		os.Exit(akisCalistir(*anahtarYol))
	}
	if *calDosya != "" {
		os.Exit(calCalistir(*calDosya, *anahtarYol))
	}
	if *hazirKip {
		n := hazirSeslerHazirla(*anahtarYol)
		fmt.Fprintf(os.Stderr, "hazır ses: %d yeni, %d eksik\n", n, hazirEksik())
		return
	}
	if *saglikYaz {
		ctx, iptal := context.WithTimeout(context.Background(), 30*time.Second)
		defer iptal()
		fmt.Println(sabahOzeti(ctx, *sehirYol))
		return
	}
	if *tgIsle != "" || *tgZamanli { // v41: sürekli döngü C'de (kole-telgraf-v1), Go tek atış
		sunucuYollar = soruYollar{anahtar: *anahtarYol, model: *modelYol, yuz: *yuzYol, gecmis: *gecmisYol, sehir: *sehirYol,
			eylem: *eylemYol, sure: *sure} // serbest metin sorusu bu süreçte (soket açılmaz)
		if *tgIsle != "" {
			os.Exit(tgIsleTek(*tgIsle, *sehirYol))
		}
		os.Exit(tgZamanliTek(*sehirYol))
	}
	if *telegramKip {
		sunucuYollar = soruYollar{anahtar: *anahtarYol, model: *modelYol, yuz: *yuzYol, gecmis: *gecmisYol, sehir: *sehirYol,
			eylem: *eylemYol, sure: *sure} // v40: soru soketi
		if err := telegramDinle(*sehirYol); err != nil {
			fmt.Fprintln(os.Stderr, "telegram:", err)
			os.Exit(9)
		}
		return
	}
	if *bildirimKip {
		if err := bildirimGonder(); err != nil {
			fmt.Fprintln(os.Stderr, "bildirim:", err)
			os.Exit(8)
		}
		return
	}
	if *veriKip {
		if veriAtla(*veriYol) { // v28: sürekli Telegram süreci Pano verisini yeniliyor
			hazirSeslerHazirla(*anahtarYol)
			return
		}
		err := veriYaz(*veriYol, *sehirYol)
		hazirSeslerHazirla(*anahtarYol) // v15: eksik hazir sesler (hepsi varsa ag kullanmaz)
		if err != nil {
			fmt.Fprintln(os.Stderr, "veri:", err)
			os.Exit(7)
		}
		return
	}
	if (*ses == "") == (*metin == "") {
		fmt.Fprintln(os.Stderr, "kullanim: --ses <wav> | --metin <soru>")
		os.Exit(2)
	}
	y := soruYollar{anahtar: *anahtarYol, model: *modelYol, yuz: *yuzYol, gecmis: *gecmisYol, sehir: *sehirYol, eylem: *eylemYol,
		sure: *sure, ayriSurec: true}
	ist := soruIstek{Ses: *ses, Metin: *metin}
	if os.Getenv("KOLE_SOKETSIZ") == "" { // v40: sürekli süreç soketteyse ona sor (ince istemci; Go yığını büyümez)
		bekle := 32 * time.Second
		if *ses == "" {
			bekle = 47 * time.Second
		}
		if out, rc, ok := soketSor(soruSoket, ist, bekle); ok {
			if rc != 0 {
				os.Exit(rc)
			}
			fmt.Println(out)
			return
		}
	}
	s, rc := soruIsle(y, ist)
	if rc != 0 {
		os.Exit(rc)
	}
	out, _ := json.Marshal(s)
	fmt.Println(string(out))
}

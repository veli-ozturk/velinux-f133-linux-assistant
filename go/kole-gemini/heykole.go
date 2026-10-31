// heykole.go (23.10.2026, Claude; kole-gemini v35): Hey Köle dinlemesini Telegram'dan aç/kapat.
// kullanıcı: "hey köle modu için açma kapama ekle". Bayrak /data/asistan/kapali: varken kole-asistan mikrofonu bırakır ve
// uyanmaz; panel 0.75 saniyede bir okuyup Denetim Merkezi düğmesini ve Ayarlar anahtarını eşitler.
package main

var heyKoleKapali = "/data/asistan/kapali"

func heyKoleKisa() string {
	if dosyaVar(heyKoleKapali) {
		return "Hey Köle kapalı"
	}
	return "Hey Köle açık"
}

func heyKoleDurum() string {
	if dosyaVar(heyKoleKapali) {
		return "🔇 Hey Köle: kapalı (mikrofon bırakıldı, \"Hey Köle\" denince uyanmaz)"
	}
	return "🎤 Hey Köle: açık (\"Hey Köle\" denince dinler)"
}

// heyKoleKomut: /heykole argümanı -> cevap
func heyKoleKomut(arg string) string {
	switch arg {
	case "ac", "aç":
		if err := bayrak(heyKoleKapali, false); err != nil {
			return "Açamadım efendim: " + err.Error()
		}
		return "🎤 Hey Köle açıldı efendim; \"Hey Köle\" deyince dinlerim."
	case "kapat":
		if err := bayrak(heyKoleKapali, true); err != nil {
			return "Kapatamadım efendim: " + err.Error()
		}
		return "🔇 Hey Köle kapatıldı efendim, mikrofon bırakıldı. Açmak için /heykole ac ya da paneldeki Denetim Merkezi."
	}
	return heyKoleDurum() + "\nKullanım: /heykole ac | kapat"
}

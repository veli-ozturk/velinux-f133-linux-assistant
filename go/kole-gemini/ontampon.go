// ontampon.go (21.10.2026, Claude; kole-gemini v24, OTONOM Sıra 7 madde 43): akışlı TTS'te köprü ne zaman açılsın.
// 20.10 22:11 ölçümü: 8.2 sn konuşmada xrun 13 (takılma) — ilk ses parçasında köprü açılıyordu; Gemini akışı gerçek
// zamandan yavaş üretince köprü (150 ms ön tampon) tekrar tekrar boşaldı.
// Model: köprü açıldığında önce Z = onSessizMs sıfır çalınır, sonra biriken B; kalan ses r hızıyla (ses ms / duvar ms)
// gelir. Takılmasız çalma için (sabit r, toplam D): B >= D*(1-r) - r*Z. Pay: x1.15 + 300 ms. r >= 1 ise hemen açılır.
// Hız ilk parçadan SONRA gelenlerle ölçülür (ilk parçanın sesi 0 sürede gelmiş sayılmasın); en çok akisOnTamponAzami
// beklenir (gecikme sınırı); akış biterse kalan çalınır.
package main

import "time"

var (
	akisOnTamponAzami  = 4 * time.Second
	akisOnTamponKapali = false // test: v23 davranışı (ilk parçada aç)
)

// onTamponYeter: tamponMs biriken ses, ilkParcaMs ilk parçanın sesi, gecen ilk parçadan beri duvar süresi, tahminMs
// toplam ses tahmini. Döner: köprü şimdi açılsın mı, ölçülen hız (0: henüz yok).
func onTamponYeter(tamponMs, ilkParcaMs int, gecen time.Duration, tahminMs int) (bool, float64) {
	if akisOnTamponKapali || gecen >= akisOnTamponAzami || tamponMs >= tahminMs {
		return true, 0
	}
	sonraki := tamponMs - ilkParcaMs
	if sonraki <= 0 || gecen < 50*time.Millisecond {
		return false, 0 // hız ölçülemedi: bir parça daha bekle
	}
	hiz := float64(sonraki) / float64(gecen.Milliseconds())
	if hiz >= 1 {
		return true, hiz
	}
	gerekli := (float64(tahminMs)*(1-hiz)-hiz*float64(onSessizMs))*1.15 + 300
	return float64(tamponMs) >= gerekli, hiz
}

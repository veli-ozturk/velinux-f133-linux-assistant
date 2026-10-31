package main

// haber_siber.go (21.10.2026, Claude; v29, scripts/490): kullanıcı: "haberlere siber güvenlik haberlerini de ekle".
// Kaynaklar (21.10 PC'den denendi: 200, son haber 1-5 saat): ShiftDelete güvenlik (Türkçe), BleepingComputer, The Hacker
// News, SecurityWeek, Dark Reading. SiberBülten (bozuk XML), USOM (boş akış), Webtekno güvenlik (404), Krebs (141 KB)
// alınmadı.
//   - Telegram /haber bülteninde "🛡 Siber güvenlik" kategorisi; /haber siber; menüde düğme.
//   - Pano: haber döngüsünün son 2'si siber (yalnız küçük iki akış: ShiftDelete 21 KB, BleepingComputer 12 KB; Telegram
//     süreci 10 dk'da bir), "Siber: " önekiyle.
//   - Hey Köle sesli haber özeti siber akışları ÇEKMEZ (İngilizce başlıklar Türkçe seste kötü okunur, gecikme artmasın).

import (
	"context"
	"strings"
	"sync"
	"time"
)

const katSiber = "siber"

var siberPanoKaynak = map[string]bool{"ShiftDelete": true, "BleepingComputer": true}

// haberOzel: teknoloji ve siber, genel kategorilerden sonra gelir (aynı olay gündemdeyse orada durur)
func haberOzel(k string) bool { return k == katTeknoloji || k == katSiber }

// haberToplaFn: haberTopla gibi, kaynak süzgeçli
func haberToplaFn(ctx context.Context, al func(haberKaynak) bool) []haberOge {
	var (
		mu  sync.Mutex
		wg  sync.WaitGroup
		hep []haberOge
	)
	for _, k := range haberKaynaklari {
		if !al(k) {
			continue
		}
		wg.Add(1)
		go func(k haberKaynak) {
			defer wg.Done()
			c, iptal := context.WithTimeout(ctx, 8*time.Second)
			defer iptal()
			o, err := haberAkis(c, k)
			if err != nil {
				return
			}
			mu.Lock()
			hep = append(hep, o...)
			mu.Unlock()
		}(k)
	}
	wg.Wait()
	return hep
}

// siberPano: Pano için en önemli n siber başlık (küçük iki akıştan)
func siberPano(ctx context.Context, n int) []string {
	var o []string
	for _, c := range haberKumele(haberToplaFn(ctx, func(k haberKaynak) bool { return k.Kat == katSiber && siberPanoKaynak[k.Ad] }),
		time.Now()) {
		if len(o) == n {
			break
		}
		o = append(o, c.Baslik)
	}
	return o
}

// panoHaberBirlestir: genel başlıklar + sonda "Siber: " önekli siber başlıklar, toplam en çok n
func panoHaberBirlestir(genel, siber []string, n int) []string {
	if len(siber) > n {
		siber = siber[:n]
	}
	g := n - len(siber)
	if g > len(genel) {
		g = len(genel)
	}
	o := append([]string{}, genel[:g]...)
	for _, s := range siber {
		o = append(o, "Siber: "+strings.TrimSpace(s))
	}
	return o
}

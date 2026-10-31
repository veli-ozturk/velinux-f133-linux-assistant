package main

// izinli.go v45 (30.10.2026, Claude; kullanıcı: eşi de bota): /data/asistan/telegram-izinli (satır başına bir sohbet
// kimliği). İzinli sohbet sahip gibi komut verir, cevap mesajı yazana gider; otomatik bildirimler sahip + izinlilere.
// kole-telgraf-v7 aynı dosyayı okur (izinsiz özel sohbetin kimliğini günlüğe yazar).

import (
	"context"
	"os"
	"slices"
	"strconv"
	"strings"
)

var telegramIzinli = "/data/asistan/telegram-izinli"

const izinliAzami = 5

// izinliler: geçerli, sahipten farklı, tekrarsız en çok izinliAzami kimlik
func izinliler(sahip string) []string {
	b, _ := os.ReadFile(telegramIzinli)
	var l []string
	for _, f := range strings.Fields(string(b)) {
		if _, err := strconv.ParseInt(f, 10, 64); err == nil && f != sahip && !slices.Contains(l, f) && len(l) < izinliAzami {
			l = append(l, f)
		}
	}
	return l
}

func yetkiliSohbet(sohbet, sahip string) bool {
	return sahip != "" && (sohbet == sahip || slices.Contains(izinliler(sahip), sohbet))
}

// tgHerkese: otomatik bildirim (sabah özeti, hizmet/bellek uyarıları) sahip + izinlilere
func tgHerkese(ctx context.Context, jeton, sahip, metin string) {
	for _, s := range append([]string{sahip}, izinliler(sahip)...) {
		telegramYanit(ctx, jeton, s, metin)
	}
}

package main

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestJpKisayol(t *testing.T) {
	if tgDugmeKomut("🗾 Gezi") != "/jp" {
		t.Fatal("düğme /jp değil")
	}
	var bulundu bool
	for _, k := range tgKomutlar {
		bulundu = bulundu || k[0] == "jp"
	}
	if !bulundu {
		t.Fatal("sol menüde jp yok")
	}
	var u tgGuncelleme
	js := `{"update_id":1,"message":{"message_id":2,"text":"12 Kasım?","chat":{"id":42},"reply_to_message":{"text":` +
		strings.ReplaceAll(mustJSON(jpSoruIstem), "\n", "") + `}}}`
	if err := json.Unmarshal([]byte(js), &u); err != nil || u.Message.Yanitlanan == nil || u.Message.Yanitlanan.Text != jpSoruIstem {
		t.Fatalf("reply_to_message çözülmedi: %v", err)
	}
	if !strings.Contains(jpTalimat, "🔗 Sayfa:") || !strings.Contains(jpTalimat, jpAnaSayfa) {
		t.Fatal("talimatta sayfa kuralı yok")
	}
}

func mustJSON(s string) string {
	b, _ := json.Marshal(s)
	return string(b)
}

// istem_test.go (22.10.2026, Claude; v31): Hey Köle kuralları genel bilgiyi doğrudan yanıtlatır, canlı veriyi uydurtmaz.
package main

import (
	"strings"
	"testing"
)

func TestIstemGenelBilgi(t *testing.T) {
	if strings.Contains(ortakTalimat, "Bilmediğin güncel bir bilgiyi uydurma, bilmediğini söyle") {
		t.Fatal("v30 kuralı duruyor: genel bilgi sorularında 'bilmiyorum' diyordu")
	}
	for _, s := range []string{"Genel bilgi sorularını", "yaklaşık", "somut bir öneri", "canlı veriyi uydurma", "tekrar etmesini iste", "%s"} {
		if !strings.Contains(ortakTalimat, s) {
			t.Fatalf("istemde yok: %q", s)
		}
	}
	if strings.Count(ortakTalimat, "%") != 1 {
		t.Fatalf("Sprintf yer tutucusu tek olmalı: %d", strings.Count(ortakTalimat, "%"))
	}
}

func TestModelSirasi(t *testing.T) {
	m := strings.Split(varsayilanModel, ",")
	if m[0] != "gemini-3.5-flash-lite" || strings.Contains(varsayilanModel, "2.5-flash-lite") {
		t.Fatalf("model sırası: %v", m)
	}
	if s := strings.Split(sesModelVarsayilan, ","); s[0] != "gemini-3.5-flash-lite" {
		t.Fatalf("sesli sıra: %v", s)
	}
}

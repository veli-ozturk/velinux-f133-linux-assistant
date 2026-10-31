// eylem_ses_test.go (22.10.2026, Claude; v32): "ses" eylemi -> panel 0.73 ui anahtarları.
package main

import (
	"reflect"
	"strings"
	"testing"
)

func TestEylemSes(t *testing.T) {
	durum := []struct {
		d string
		u map[string]any
	}{
		{"40", map[string]any{"ses": 40}}, {"%70", map[string]any{"ses": 70}}, {"150", map[string]any{"ses": 100}},
		{"-20", map[string]any{"ses_rel": -20}}, {"+40", map[string]any{"ses_rel": 40}}, {"+500", map[string]any{"ses_rel": 100}},
		{"sessiz", map[string]any{"sessiz": true}}, {"Kapat", map[string]any{"sessiz": true}}, {"ac", map[string]any{"sessiz": false}},
		{"+0", nil}, {"biraz", nil}, {"", nil},
	}
	for _, x := range durum {
		if u := eylemUi(eylem{Ad: "ses", Deger: x.d}); !reflect.DeepEqual(u, x.u) {
			t.Fatalf("ses %q: %v (beklenen %v)", x.d, u, x.u)
		}
	}
	if !strings.Contains(eylemTalimat, "PANELİN KENDİ hoparlör sesi") || eylemAdlari[1] != "ses" {
		t.Fatal("istem/enum")
	}
	if u := eylemUi(eylem{Ad: "muzik", Deger: "ses_kis"}); !reflect.DeepEqual(u, map[string]any{"press": "macro:voldown"}) {
		t.Fatalf("muzik ses_kis bozuldu: %v", u)
	}
}

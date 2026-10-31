package main

import "testing"

func TestParlaklik(t *testing.T) {
	cases := map[string]string{"30": "br_pct", "+20": "br_rel", "-40": "br_rel", "oto": "br_mode", "abc": ""}
	for d, k := range cases {
		u := eylemUi(eylem{Ad: "parlaklik", Deger: d})
		if k == "" {
			if u != nil {
				t.Errorf("%s: gecersiz kabul edildi %v", d, u)
			}
			continue
		}
		if _, ok := u[k]; !ok {
			t.Errorf("%s: %s yok (%v)", d, k, u)
		}
	}
}

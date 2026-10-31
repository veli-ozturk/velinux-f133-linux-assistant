package main

import (
	"testing"
	"time"
)

func TestHatirlatici(t *testing.T) {
	time.Local = trZaman
	z, m, err := hatirlaticiCoz("2030-01-02 09:30 | ilaç iç")
	if err != nil || m != "ilaç iç" || time.Unix(z, 0).In(trZaman).Format("2006-01-02 15:04") != "2030-01-02 09:30" {
		t.Fatalf("coz: %v %q %v", z, m, err)
	}
	if u := eylemUi(eylem{Ad: "alarm", Deger: "her gün 07:00"}); u == nil {
		t.Fatal("her gun alarm reddedildi")
	}
	if u := eylemUi(eylem{Ad: "hatirlatici", Deger: "2001-01-01 09:00 | gecmis"}); u != nil {
		t.Fatal("gecmis zaman kabul edildi")
	}
	if u := eylemUi(eylem{Ad: "ekran_koruyucu", Deger: "cerceve"}); u == nil || u["ss"] != "cerceve" {
		t.Fatal("cerceve eylemi")
	}
	if trTarih(time.Date(2026, 10, 16, 9, 5, 0, 0, trZaman)) != "16 Ekim 09:05" {
		t.Fatal("trTarih")
	}
}

package main

import (
	"os"
	"path/filepath"
	"slices"
	"testing"
)

func TestIzinliler(t *testing.T) {
	eski := telegramIzinli
	defer func() { telegramIzinli = eski }()
	telegramIzinli = filepath.Join(t.TempDir(), "izinli")
	if len(izinliler("42")) != 0 || !yetkiliSohbet("42", "42") || yetkiliSohbet("77", "42") {
		t.Fatal("dosya yokken yalnız sahip")
	}
	_ = os.WriteFile(telegramIzinli, []byte("77\n42\nabc\n -88 \n77\n"), 0600)
	if l := izinliler("42"); !slices.Equal(l, []string{"77", "-88"}) {
		t.Fatalf("izinliler %v", l)
	}
	if !yetkiliSohbet("77", "42") || !yetkiliSohbet("-88", "42") || yetkiliSohbet("99", "42") || yetkiliSohbet("77", "") {
		t.Fatal("yetki")
	}
}

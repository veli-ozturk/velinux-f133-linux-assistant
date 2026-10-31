// eylem_enum_test.go (22.10.2026, Claude; v33)
package main

import (
	"slices"
	"testing"
)

func TestEylemEnum(t *testing.T) {
	if e := eylemEnum(false); !slices.Contains(e, "haber") || len(e) != len(eylemAdlari) {
		t.Fatalf("bağlamsız: %v", e)
	}
	e := eylemEnum(true)
	if slices.Contains(e, "haber") || len(e) != len(eylemAdlari)-1 || !slices.Contains(e, "ses") {
		t.Fatalf("bağlamlı: %v", e)
	}
	if !slices.Contains(eylemAdlari, "haber") {
		t.Fatal("eylemAdlari değişmemeli")
	}
}

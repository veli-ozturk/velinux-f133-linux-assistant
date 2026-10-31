package main

import (
	"bytes"
	"image"
	"image/color"
	"image/jpeg"
	"image/png"
	"os"
	"path/filepath"
	"testing"
	"time"
)

func ornekResim() image.Image {
	m := image.NewRGBA(image.Rect(0, 0, 64, 48))
	for y := 0; y < 48; y++ {
		for x := 0; x < 64; x++ {
			m.Set(x, y, color.RGBA{uint8(x * 4), uint8(y * 5), 90, 255})
		}
	}
	return m
}

func TestJpegTuru(t *testing.T) {
	var g bytes.Buffer
	_ = jpeg.Encode(&g, ornekResim(), nil)
	if v := jpegTuru(g.Bytes()); v != "baseline" {
		t.Fatalf("go jpeg: %q", v)
	}
	// elle progresif başlık: SOI, APP0 (boş), SOF2
	prog := []byte{0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x04, 0x00, 0x00, 0xFF, 0xC2, 0x00, 0x0B, 8, 0, 48, 0, 64, 1, 1, 0x11, 0}
	if v := jpegTuru(prog); v != "progresif" {
		t.Fatalf("progresif: %q", v)
	}
	if jpegTuru([]byte("\x89PNG....")) != "" || jpegTuru([]byte{0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x01}) != "" {
		t.Fatal("bozuk veri kabul edildi")
	}
}

func TestCerceveKaydet(t *testing.T) {
	d := t.TempDir()
	var p bytes.Buffer
	_ = png.Encode(&p, ornekResim())
	z := time.Date(2026, 10, 15, 9, 0, 0, 0, time.UTC)
	y1, islem, err := cerceveKaydet(d, p.Bytes(), z)
	if err != nil || islem != "png->baseline" || filepath.Base(y1) != "tg-20261015-090000.jpg" {
		t.Fatalf("png: %v %q %q", err, islem, y1)
	}
	b, _ := os.ReadFile(y1)
	if jpegTuru(b) != "baseline" {
		t.Fatal("cikti baseline degil")
	}
	if _, err := jpeg.Decode(bytes.NewReader(b)); err != nil {
		t.Fatal(err)
	}
	var j bytes.Buffer
	_ = jpeg.Encode(&j, ornekResim(), nil)
	y2, islem, err := cerceveKaydet(d, j.Bytes(), z)
	if err != nil || islem != "baseline" || filepath.Base(y2) != "tg-20261015-090000-2.jpg" {
		t.Fatalf("ikinci: %v %q %q", err, islem, y2)
	}
	if b2, _ := os.ReadFile(y2); !bytes.Equal(b2, j.Bytes()) {
		t.Fatal("baseline degistirildi")
	}
	if _, _, err := cerceveKaydet(d, []byte("metin"), z); err == nil {
		t.Fatal("metin kabul edildi")
	}
	var buyuk bytes.Buffer
	_ = png.Encode(&buyuk, image.NewGray(image.Rect(0, 0, 2000, 2000)))
	if _, _, err := cerceveKaydet(d, buyuk.Bytes(), z); err == nil {
		t.Fatal("4 MP PNG kabul edildi")
	}
	if n := cerceveSay(d); n != 2 {
		t.Fatalf("sayi %d", n)
	}
	es, _ := os.ReadDir(d)
	for _, e := range es {
		if filepath.Ext(e.Name()) == ".part" {
			t.Fatal(".part kaldi")
		}
	}
}

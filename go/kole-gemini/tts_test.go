package main

import (
	"bufio"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"io"
	"math"
	"net"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func sinus(hz, f float64, n int, gen float64) []int16 {
	x := make([]int16, n)
	for i := range x {
		x[i] = int16(gen * math.Sin(2*math.Pi*f*float64(i)/hz))
	}
	return x
}

func rms(x []int16) float64 {
	s := 0.0
	for _, v := range x {
		s += float64(v) * float64(v)
	}
	return math.Sqrt(s / float64(len(x)))
}

func TestYenidenOrnekle(t *testing.T) {
	x := sinus(24000, 1000, 24000, 10000)
	y := yenidenOrnekle(x, 24000, 16000)
	if len(y) != 16000 {
		t.Fatalf("uzunluk %d", len(y))
	}
	g := y[200 : len(y)-200]
	if r := rms(g) / rms(x); r < 0.97 || r > 1.03 {
		t.Fatalf("genlik orani %.3f", r)
	}
	gecis := 0
	for i := 1; i < len(g); i++ {
		if (g[i-1] < 0) != (g[i] < 0) {
			gecis++
		}
	}
	if f := float64(gecis) / 2 / (float64(len(g)) / 16000); f < 990 || f > 1010 { // 1 kHz korunur
		t.Fatalf("frekans %.0f", f)
	}
	yuksek := yenidenOrnekle(sinus(24000, 11000, 24000, 10000), 24000, 16000) // 8 kHz ustu suzulur
	if r := rms(yuksek[200 : len(yuksek)-200]); r > 400 {
		t.Fatalf("11 kHz suzulmedi: rms %.0f", r)
	}
	if len(yenidenOrnekle(x[:100], 16000, 16000)) != 100 {
		t.Fatal("ayni hiz")
	}
}

func pcmB64(x []int16) string {
	b := make([]byte, 2*len(x))
	for i, v := range x {
		binary.LittleEndian.PutUint16(b[2*i:], uint16(v))
	}
	return base64.StdEncoding.EncodeToString(b)
}

func TestSesCoz(t *testing.T) {
	x := []int16{1, -2, 300, -400}
	j := `{"steps":[{"type":"model_output","content":[{"type":"text","text":"x"},{"type":"audio","mime_type":"audio/l16;rate=24000","channels":1,"data":"` + pcmB64(x) + `"}]}]}`
	y, hz, err := ttsSesCoz([]byte(j))
	if err != nil || hz != 24000 || len(y) != 4 || y[2] != 300 || y[3] != -400 {
		t.Fatalf("l16: %v %d %v", err, hz, y)
	}
	j2 := `{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16","sample_rate":16000,"data":"` + pcmB64(x) + `"}]}]}`
	if _, hz, err := ttsSesCoz([]byte(j2)); err != nil || hz != 16000 {
		t.Fatalf("sample_rate alani: %v %d", err, hz)
	}
	wav := make([]byte, 44+8)
	copy(wav, "RIFF")
	copy(wav[8:], "WAVEfmt ")
	binary.LittleEndian.PutUint32(wav[16:], 16)
	binary.LittleEndian.PutUint16(wav[20:], 1)
	binary.LittleEndian.PutUint16(wav[22:], 1)
	binary.LittleEndian.PutUint32(wav[24:], 24000)
	binary.LittleEndian.PutUint16(wav[34:], 16)
	copy(wav[36:], "data")
	binary.LittleEndian.PutUint32(wav[40:], 8)
	copy(wav[44:], func() []byte { b, _ := base64.StdEncoding.DecodeString(pcmB64(x)); return b }())
	j3 := `{"steps":[{"content":[{"type":"audio","mime_type":"audio/wav","data":"` + base64.StdEncoding.EncodeToString(wav) + `"}]}]}`
	if y, hz, err := ttsSesCoz([]byte(j3)); err != nil || hz != 24000 || len(y) != 4 {
		t.Fatalf("wav: %v %d %v", err, hz, y)
	}
	if _, _, err := ttsSesCoz([]byte(`{"steps":[]}`)); err == nil {
		t.Fatal("sessiz yanit hata vermeli")
	}
	if _, _, err := ttsSesCoz([]byte(`{"steps":[{"content":[{"type":"audio","channels":2,"data":"AAAA"}]}]}`)); err == nil {
		t.Fatal("stereo reddedilmeli")
	}
	if _, _, err := ttsSesCoz([]byte(`{"error":{"message":"kota"}}`)); err == nil || err.Error() != "kota" {
		t.Fatal("hata mesaji")
	}
}

func TestGovde(t *testing.T) {
	var g map[string]any
	if err := json.Unmarshal(ttsGovde("gemini-3.8-flash-tts", "Merhaba efendim", "Puck", "genç"), &g); err != nil {
		t.Fatal(err)
	}
	s, _ := json.Marshal(g)
	for _, p := range []string{`"model":"gemini-3.8-flash-tts"`, `"type":"user_input"`, `"text":"Merhaba efendim"`,
		`"type":"speech_metadata"`, `"style":"genç"`, `"voice":"Puck"`, `"mime_type":"audio/l16"`} {
		if !strings.Contains(string(s), p) {
			t.Fatalf("govdede yok: %s\n%s", p, s)
		}
	}
	if a, ok := ttsSesBul(" leda "); !ok || a != "Leda" {
		t.Fatal("ses bul")
	}
	if _, ok := ttsSesBul("yok"); ok {
		t.Fatal("bilinmeyen ses")
	}
}

func TestKopruCal(t *testing.T) {
	d := t.TempDir()
	kopruAnahtarYol = filepath.Join(d, "anahtar")
	_ = os.WriteFile(kopruAnahtarYol, []byte("GIZLI123\n"), 0600)
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer l.Close()
	kopruAdres = l.Addr().String()
	gelen := make(chan string, 1)
	go func() {
		c, err := l.Accept()
		if err != nil {
			return
		}
		r := bufio.NewReader(c)
		bas, _ := r.ReadString('\n')
		var n int
		parca := strings.Fields(bas)
		if len(parca) == 3 {
			n = atoiTest(parca[2])
		}
		b := make([]byte, n)
		_, _ = io.ReadFull(r, b)
		sessiz := true
		for _, v := range b[:32000] {
			if v != 0 {
				sessiz = false
			}
		}
		gelen <- strings.TrimSpace(bas) + map[bool]string{true: " onsessiz", false: " onsesli"}[sessiz]
		c.Write([]byte("OK 0\n"))
		c.Close()
	}()
	x := sinus(16000, 440, 1600, 3000)
	if err := kopruCal(x); err != nil {
		t.Fatal(err)
	}
	beklenen := "GIZLI123 PLAY " + itoaTest(2*(16000+1600+3200)) + " onsessiz"
	if g := <-gelen; g != beklenen {
		t.Fatalf("baslik %q, beklenen %q", g, beklenen)
	}
}

func atoiTest(s string) int {
	n := 0
	for _, c := range s {
		n = n*10 + int(c-'0')
	}
	return n
}

func itoaTest(n int) string {
	if n == 0 {
		return "0"
	}
	s := ""
	for n > 0 {
		s = string(rune('0'+n%10)) + s
		n /= 10
	}
	return s
}

func TestMikBirak(t *testing.T) {
	d := t.TempDir()
	mikBirakYol = filepath.Join(d, "mik-birak")
	if !mikBirakAl() || mikBirakAl() {
		t.Fatal("ilk alim olmali, ikinci olmamali")
	}
	mikBirakBirak()
	if _, err := os.Stat(mikBirakYol); err == nil {
		t.Fatal("bizim bayrak silinmeli")
	}
	_ = os.WriteFile(mikBirakYol, []byte("kopru 12\n"), 0644) // kole-kopru'nun bayragi
	if mikBirakAl() {
		t.Fatal("baskasinin bayragi varken alinmamali")
	}
	mikBirakBirak()
	if _, err := os.Stat(mikBirakYol); err != nil {
		t.Fatal("baskasinin bayragi silinmemeli")
	}
}

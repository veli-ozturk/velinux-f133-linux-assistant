package main

import (
	"context"
	"errors"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"
)

func TestLlmIstek(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.Header.Get("x-goog-api-key") != "K" {
			w.WriteHeader(401)
			return
		}
		switch r.URL.Path {
		case "/yavas-baslik":
			time.Sleep(1500 * time.Millisecond)
		case "/yavas-govde":
			w.WriteHeader(200)
			_, _ = w.Write([]byte(`{"candidates":[`))
			w.(http.Flusher).Flush()
			time.Sleep(1500 * time.Millisecond)
			_, _ = w.Write([]byte(`]}`))
			return
		}
		w.WriteHeader(503)
		_, _ = w.Write([]byte(`{"error":{"message":"high demand"}}`))
	}))
	defer srv.Close()
	ctx, iptal := context.WithTimeout(context.Background(), 5*time.Second)
	defer iptal()
	for _, yol := range []string{"/yavas-baslik", "/yavas-govde"} {
		t0 := time.Now()
		_, _, err := llmIstek(ctx, srv.URL+yol, nil, "K", 300*time.Millisecond)
		if !errors.Is(err, errModelSure) || ctx.Err() != nil || time.Since(t0) > time.Second {
			t.Fatalf("%s: model sinirda kesilmeli, genel baglam surmeli: %v %v", yol, err, time.Since(t0))
		}
	}
	res, b, err := llmIstek(ctx, srv.URL+"/hizli", nil, "K", 2*time.Second)
	if err != nil || res.StatusCode != 503 || string(b) != `{"error":{"message":"high demand"}}` {
		t.Fatalf("503 govdesi okunmali: %v %v %s", err, res, b)
	}
	// genel sure dolarsa errModelSure DEGIL (cagiran "zaman asimi" ile cikar)
	kisa, kiptal := context.WithTimeout(context.Background(), 200*time.Millisecond)
	defer kiptal()
	_, _, err = llmIstek(kisa, srv.URL+"/yavas-baslik", nil, "K", time.Second)
	if err == nil || errors.Is(err, errModelSure) || !errors.Is(err, context.DeadlineExceeded) {
		t.Fatalf("genel sure dolmasi: %v", err)
	}
}

// model_sure.go (20.10.2026, Claude): tek model isteği kendi süresiyle (v19, scripts/392; v20'de llmYaris kullanır).
package main

import (
	"bytes"
	"context"
	"errors"
	"io"
	"net/http"
	"time"
)

var llmURLTaban = "https://generativelanguage.googleapis.com/v1beta/models/"

// genel süre sürerken yalnız bu isteğin süresi doldu
var errModelSure = errors.New("model suresi doldu")

// tek model isteği kendi süresiyle; gövde de bu süre içinde okunur. Dönen yanıtın gövdesi kapalıdır (b'de).
func llmIstek(ctx context.Context, url string, govde []byte, anahtar string, sure time.Duration) (*http.Response, []byte, error) {
	mctx, iptal := context.WithTimeout(ctx, sure)
	defer iptal()
	req, err := http.NewRequestWithContext(mctx, http.MethodPost, url, bytes.NewReader(govde))
	if err != nil {
		return nil, nil, err
	}
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("x-goog-api-key", anahtar)
	res, err := http.DefaultClient.Do(req)
	var b []byte
	if err == nil {
		b, err = io.ReadAll(io.LimitReader(res.Body, 1<<20))
		res.Body.Close()
	}
	if err != nil {
		if ctx.Err() == nil && errors.Is(mctx.Err(), context.DeadlineExceeded) {
			return nil, nil, errModelSure
		}
		return nil, nil, err
	}
	return res, b, nil
}

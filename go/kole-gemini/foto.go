// foto.go (15.10.2026, Claude): Telegram sendPhoto (multipart). Kamera hareket bildirimi kucuk JPEG gonderir.
package main

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"mime/multipart"
	"net/http"
	"os"
	"strings"
)

func telegramFoto(ctx context.Context, jeton, sohbet, yol, baslik string) error {
	jpg, err := os.ReadFile(yol)
	if err != nil {
		return err
	}
	if len(jpg) > 5<<20 {
		return fmt.Errorf("foto cok buyuk")
	}
	var g bytes.Buffer
	w := multipart.NewWriter(&g)
	_ = w.WriteField("chat_id", sohbet)
	if baslik != "" {
		_ = w.WriteField("caption", baslik)
	}
	p, _ := w.CreateFormFile("photo", "kamera.jpg")
	_, _ = p.Write(jpg)
	_ = w.Close()
	req, _ := http.NewRequestWithContext(ctx, http.MethodPost, tgTaban+"/bot"+jeton+"/sendPhoto", &g)
	req.Header.Set("Content-Type", w.FormDataContentType())
	res, err := http.DefaultClient.Do(req)
	if err != nil {
		return fmt.Errorf("%s", strings.ReplaceAll(err.Error(), jeton, "***"))
	}
	defer res.Body.Close()
	b, _ := io.ReadAll(io.LimitReader(res.Body, 1<<16))
	if res.StatusCode != 200 {
		return fmt.Errorf("telegram sendPhoto HTTP %d %s", res.StatusCode, strings.ReplaceAll(string(b), jeton, "***"))
	}
	return nil
}

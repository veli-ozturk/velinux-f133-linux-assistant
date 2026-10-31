#ifndef PC_SESSION_V2_H
#define PC_SESSION_V2_H
/* pc_session_v2 (21.10.2026, Claude; panel 0.65, scripts/494): v1 + baglanma kipi. kullanıcı: "pc ekrani sayfasina gelince
 * otomatik baglanmasin, manual veya otomatik mod secimi olsun". oto=0 (Elle, varsayilan): sekmeye girmek baglamaz, yalniz
 * pc_session_connect (Baglan / Tam ekran). oto=1: v1 ile ayni (sekmeye girince baglanir). Ayrilinca (tam ekran degilse)
 * iki kipte de birakir. --- v1 ---
 * Intent is independent of stale SHM/video. Only UI entry or explicit connect
 * sets it. Fullscreen return alone is not disconnect; dedicated close is. */
typedef struct { int want, previous_tab, blocked; } pc_session_t;
static inline void pc_session_tick(pc_session_t *s, int tab, int full, int oto) {
    if (!tab) s->blocked=0;
    if (tab && !s->previous_tab && !s->blocked && oto) s->want=1;
    if (!tab && !full) s->want=0;
    s->previous_tab=tab;
}
static inline void pc_session_connect(pc_session_t *s) { s->want=1; s->blocked=0; }
static inline void pc_session_disconnect(pc_session_t *s) { s->want=0; s->blocked=1; }
#endif

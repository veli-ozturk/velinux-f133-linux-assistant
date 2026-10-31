/* Immutable, atomically published RAM RGB888 frames; no network or hardware. */
#ifndef PC_FRAME_V2_H
#define PC_FRAME_V2_H
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define PC_FRAME_MAGIC565 0x56465232u
#define PC_FRAME_MAGIC 0x56465231u /* VFR1 */
#define PC_FRAME_MAX_W 1600u
#define PC_FRAME_MAX_H 600u
#define PC_FRAME_MAX_AGE 2000u
typedef struct { uint32_t seq, stamp, w, h, size, bpp; uint8_t *rgb; } pc_frame_t;
static uint32_t pc_frame_be(const uint8_t *p)
{ return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static uint32_t pc_frame_hash(const uint8_t *p, size_t n)
{ uint32_t h=2166136261u; for (size_t i=0;i<n;i++) h=(h^p[i])*16777619u; return h; }
/* Pinned LVGL TJpgDec already publishes B,G,R, matching LVGL RGB888. */
static int pc_frame_exact(int fd, void *buffer, size_t n)
{
    uint8_t *p=buffer;
    while (n) { ssize_t r=read(fd,p,n); if (r<0 && errno==EINTR) continue;
        if (r<=0) return 0;
        p+=r; n-=(size_t)r;
    }
    return 1;
}
/* -1 invalid/stale/missing, 0 unchanged, 1 owns new rgb allocation. */
static int pc_frame_read(const char *path, uint32_t now, uint32_t last_seq,
                         uint32_t last_stamp, pc_frame_t *out)
{
    int fd=open(path,O_RDONLY|O_NONBLOCK|O_NOFOLLOW);
    if (fd<0) return -1;
    struct stat st;
    uint8_t header[28];
    int result=-1;
    if (fstat(fd,&st) || !S_ISREG(st.st_mode) || !pc_frame_exact(fd,header,sizeof header)) goto end;
    uint32_t magic=pc_frame_be(header), seq=pc_frame_be(header+4);
    uint32_t w=pc_frame_be(header+8), h=pc_frame_be(header+12), n=pc_frame_be(header+16);
    uint32_t stamp=pc_frame_be(header+20), hash=pc_frame_be(header+24);
    uint32_t bpp=magic==PC_FRAME_MAGIC565 ? 2 : 3;
    if ((magic!=PC_FRAME_MAGIC && magic!=PC_FRAME_MAGIC565) ||
        (magic==PC_FRAME_MAGIC && (w>800 || h>300)) || !w || !h || w>PC_FRAME_MAX_W || h>PC_FRAME_MAX_H ||
        n!=w*h*bpp || st.st_size!=(off_t)(sizeof header+n) || (uint32_t)(now-stamp)>PC_FRAME_MAX_AGE) goto end;
    if (seq==last_seq && stamp==last_stamp) { result=0; goto end; }
    uint8_t *rgb=malloc(n);
    if (!rgb) goto end;
    if (!pc_frame_exact(fd,rgb,n) || pc_frame_hash(rgb,n)!=hash) { free(rgb); goto end; }
    *out=(pc_frame_t){.seq=seq,.stamp=stamp,.w=w,.h=h,.size=n,.bpp=bpp,.rgb=rgb};
    result=1;
end:
    close(fd);
    return result;
}
#endif

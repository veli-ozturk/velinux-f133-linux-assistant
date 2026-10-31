/* Host-only link wrappers: exercise complete tone loop without opening hardware. */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sound/asound.h>

extern int ses_main(int argc, char **argv);
static int scenario, opened, closed, writes, drained;
static unsigned long accepted;
static double now;
int __wrap_open(const char *path, int flags, ...)
{
    assert(!strcmp(path, "/dev/snd/pcmC0D0p"));
    assert(flags & O_NONBLOCK);
    opened++; return 999;
}
int __wrap_close(int fd) { assert(fd == 999); closed++; return 0; }
int __wrap_clock_gettime(clockid_t clock, struct timespec *t)
{
    assert(clock == CLOCK_MONOTONIC);
    now += 0.02; t->tv_sec = (time_t)now; t->tv_nsec = (long)((now - t->tv_sec)*1e9);
    return 0;
}
int __wrap_nanosleep(const struct timespec *a, struct timespec *b)
{ (void)a; (void)b; return 0; }
int __wrap_ioctl(int fd, unsigned long request, ...)
{
    assert(fd == 999);
    if (request == SNDRV_PCM_IOCTL_HW_PARAMS || request == SNDRV_PCM_IOCTL_SW_PARAMS ||
        request == SNDRV_PCM_IOCTL_PREPARE || request == SNDRV_PCM_IOCTL_DROP) return 0;
    if (request == SNDRV_PCM_IOCTL_DRAIN) {
        drained++;
        if (scenario == 5) { errno = EIO; return -1; }
        errno = EAGAIN; return -1;
    }
    va_list ap; va_start(ap, request); void *ptr = va_arg(ap, void*); va_end(ap);
    if (request == SNDRV_PCM_IOCTL_STATUS) {
        ((struct snd_pcm_status*)ptr)->state = SNDRV_PCM_STATE_SETUP; return 0;
    }
    assert(request == SNDRV_PCM_IOCTL_WRITEI_FRAMES);
    struct snd_xferi *x = ptr;
    writes++;
    if (scenario == 1) { x->result = -EINVAL; return 0; }
    if (scenario == 2) { x->result = (snd_pcm_sframes_t)x->frames + 1; return 0; }
    if (scenario == 3) { x->result = -EPIPE; return 0; }
    if (scenario == 4) { x->result = -EAGAIN; return 0; }
    if (writes == 2) { x->result = -EAGAIN; return 0; }
    if (writes == 3) { x->result = 0; return 0; }
    unsigned n = writes == 1 ? 17 : x->frames;
    assert(n && n <= x->frames);
    const int16_t *p = x->buf;
    for (unsigned i=0; i<n; i++) {
        int expected = (int16_t)(sin(2*3.14159265358979323846*1000*(accepted+i)/48000)*32767*0.03);
        assert(p[2*i] == expected && p[2*i+1] == expected);
    }
    x->result = n; accepted += n; return 0;
}
static int run(int which)
{
    scenario=which; opened=closed=writes=drained=0; accepted=0; now=0;
    char *args[] = {"ses", "ton", "0", "0", "1000", "0.1", "3"};
    int rc = ses_main(7,args);
    assert(opened==1 && closed==1);
    return rc;
}
int main(void)
{
    assert(run(0)==0 && accepted==4800 && drained==1);
    for (int i=1; i<=4; i++) assert(run(i)==1 && drained==0);
    assert(run(5)==1 && accepted==4800 && drained==1);
    const char *bad[] = {"nan", "inf", "0", "-1", "40", "3junk", ""};
    for (unsigned i=0; i<sizeof bad/sizeof *bad; i++) {
        int before=opened;
        char *a[] = {"ses", "ton", "0", "0", "1000", "0.1", (char*)bad[i]};
        assert(ses_main(7,a)==2 && before==opened);
    }
    puts("OUTPUT TEST PASS: exact frames, partial offset/phase, retry, negative result, oversized result, xrun, deadline, drain failure, invalid input; no hardware opened");
    return 0;
}

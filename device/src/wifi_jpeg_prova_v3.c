/* v3: bounded 1600x600 JPEG receiver publishes little-endian RGB565 RAM frames.
 * No framebuffer, UART, SD or flash access. v1 preserved. */
#define _POSIX_C_SOURCE 200809L
#include "tjpgd.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define MAX_JPEG (512u * 1024u)
#define MAX_W 1600
#define MAX_H 600
static unsigned char jpeg[MAX_JPEG], pool[32768];
static unsigned char frame[MAX_W*MAX_H*2];
static volatile sig_atomic_t stopped;
static double deadline;
struct stream { size_t size, offset, pixels; uint32_t sum; };
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec+t.tv_nsec/1e9; }
static void stop(int sig) { (void)sig; stopped=1; }
static size_t input(JDEC *jd, uint8_t *out, size_t n)
{
    struct stream *s=jd->device;
    if (n>s->size-s->offset) n=s->size-s->offset;
    if (out) memcpy(out,jpeg+s->offset,n);
    s->offset+=n;
    return n;
}
static int output(JDEC *jd, void *rgb, JRECT *r)
{
    struct stream *s=jd->device;
    if (r->left>r->right || r->top>r->bottom || r->right>=jd->width || r->bottom>=jd->height) return 0;
    size_t pixels=(size_t)(r->right-r->left+1)*(r->bottom-r->top+1);
    if (s->pixels+pixels>(size_t)jd->width*jd->height) return 0;
    s->pixels+=pixels;
    const uint8_t *p=rgb;
    size_t tile_w=r->right-r->left+1;
    for (unsigned y=r->top;y<=r->bottom;y++) {
        for (unsigned x=r->left;x<=r->right;x++) {
            size_t i=((size_t)(y-r->top)*tile_w+x-r->left)*3;
            uint16_t pixel=(uint16_t)(((p[i+2]&0xf8)<<8)|((p[i+1]&0xfc)<<3)|(p[i]>>3));
            size_t pos=((size_t)y*jd->width+x)*2;
            frame[pos]=(uint8_t)pixel; frame[pos+1]=(uint8_t)(pixel>>8);
        }
    }
    return !stopped && now()<deadline;
}
static uint32_t fnv(size_t size)
{
    uint32_t h=2166136261u;
    for (size_t i=0;i<size;i++) h=(h^jpeg[i])*16777619u;
    return h;
}
static int decode(size_t size, uint32_t *w, uint32_t *h, uint32_t *usec)
{
    struct stream s={.size=size};
    JDEC jd;
    double start=now();
    int result=jd_prepare(&jd,input,pool,sizeof pool,&s);
    *w=*h=0;
    if (!result) {
        *w=jd.width; *h=jd.height;
        if (!jd.width || !jd.height || jd.width>MAX_W || jd.height>MAX_H) result=100;
        else {
            result=jd_decomp(&jd,output,0);
            if (!result && s.pixels!=(size_t)jd.width*jd.height) result=101;
        }
    }
    *usec=(uint32_t)((now()-start)*1e6);
    return result;
}
static int write_all(int fd, const void *buffer, size_t n)
{
    const unsigned char *p=buffer;
    while (n) { ssize_t r=write(fd,p,n); if (r<0 && errno==EINTR) continue;
        if (r<=0) return 0;
        p+=r; n-=(size_t)r;
    }
    return 1;
}
static int publish(uint32_t seq, uint32_t w, uint32_t h)
{
    size_t n=(size_t)w*h*2;
    uint32_t hash=2166136261u;
    for (size_t i=0;i<n;i++) hash=(hash^frame[i])*16777619u;
    uint32_t header[7]={0x56465232u,seq,w,h,(uint32_t)n,(uint32_t)(now()*1000),hash};
    for (unsigned i=0;i<7;i++) header[i]=htonl(header[i]);
    int fd=open("/tmp/pc-ekran.frame.part",O_WRONLY|O_CREAT|O_TRUNC|O_NOFOLLOW,0600);
    if (fd<0) return 0;
    int ok=write_all(fd,header,sizeof header) && write_all(fd,frame,n);
    if (close(fd)) ok=0;
    if (ok) ok=rename("/tmp/pc-ekran.frame.part","/tmp/pc-ekran.frame")==0;
    if (!ok) unlink("/tmp/pc-ekran.frame.part");
    return ok;
}
static int waitfd(int fd, short events, double expires)
{
    while (!stopped && now()<deadline && now()<expires) {
        struct pollfd p={.fd=fd,.events=events};
        int r=poll(&p,1,100);
        if (r>0) return !!(p.revents&events);
        if (r<0 && errno!=EINTR) return 0;
    }
    return 0;
}
static int receive(int fd, void *buf, size_t size)
{
    size_t offset=0;
    double expires=now()+3;
    while (offset<size && now()<expires && waitfd(fd,POLLIN,expires)) {
        ssize_t n=recv(fd,(char *)buf+offset,size-offset,MSG_DONTWAIT);
        if (n>0) offset+=(size_t)n;
        else if (n==0 || (errno!=EINTR && errno!=EAGAIN && errno!=EWOULDBLOCK)) break;
    }
    return offset==size;
}
static int reply(int fd, uint32_t *words)
{
    uint32_t wire[7];
    for (unsigned i=0;i<7;i++) wire[i]=htonl(words[i]);
    size_t offset=0;
    while (offset<sizeof wire && waitfd(fd,POLLOUT,deadline)) {
        ssize_t n=send(fd,(char *)wire+offset,sizeof wire-offset,MSG_NOSIGNAL|MSG_DONTWAIT);
        if (n>0) offset+=(size_t)n;
        else if (n==0 || (errno!=EINTR && errno!=EAGAIN && errno!=EWOULDBLOCK)) break;
    }
    return offset==sizeof wire;
}
int main(int argc, char **argv)
{
    signal(SIGINT,stop); signal(SIGTERM,stop); signal(SIGPIPE,SIG_IGN);
    deadline=now()+60;
    if (argc==3 && !strcmp(argv[1],"--file")) {
        FILE *f=fopen(argv[2],"rb");
        if (!f) return 2;
        size_t n=fread(jpeg,1,sizeof jpeg,f);
        int extra=fgetc(f); fclose(f);
        if (!n || extra!=EOF) return 2;
        uint32_t w,h,us;
        int r=decode(n,&w,&h,&us);
        printf("status=%d width=%u height=%u decode_us=%u\n",r,w,h,us);
        return r?1:0;
    }
    int port=8766, seconds=45;
    if (argc==5 && !strcmp(argv[1],"--port") && !strcmp(argv[3],"--seconds")) {
        port=atoi(argv[2]); seconds=atoi(argv[4]);
    } else if (argc!=1) return 2;
    if (port<1024 || port>65535 || seconds<5 || seconds>180) return 2;
    deadline=now()+seconds;
    int ls=socket(AF_INET,SOCK_STREAM,0), one=1;
    if (ls<0) return 2;
    setsockopt(ls,SOL_SOCKET,SO_REUSEADDR,&one,sizeof one);
    struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons((uint16_t)port)};
    if (bind(ls,(struct sockaddr *)&a,sizeof a) || listen(ls,1)) { perror("listen"); close(ls); return 2; }
    printf("JPEG Wi-Fi RAM benchmark port=%d seconds=%d max=%dx%d buffer=%u\n",port,seconds,MAX_W,MAX_H,MAX_JPEG);
    fflush(stdout);
    unsigned frames=0, errors=0;
    uint64_t total_us=0, bytes=0;
    while (!stopped && now()<deadline && waitfd(ls,POLLIN,deadline)) {
        int c=accept(ls,NULL,NULL);
        if (c<0) continue;
        while (!stopped && now()<deadline) {
            uint32_t header[3];
            if (!receive(c,header,sizeof header)) break;
            uint32_t magic=ntohl(header[0]), seq=ntohl(header[1]), size=ntohl(header[2]);
            if (magic!=0x564a3031u || !size || size>MAX_JPEG) { errors++; break; }
            if (!receive(c,jpeg,size)) { errors++; break; }
            uint32_t w,h,us;
            int r=decode(size,&w,&h,&us);
            if (!r && !publish(seq,w,h)) r=102;
            uint32_t ack[7]={0x564a4131u,seq,(uint32_t)r,w,h,us,fnv(size)};
            if (!r) { frames++; total_us+=us; bytes+=size; } else errors++;
            if (!reply(c,ack)) break;
        }
        close(c);
    }
    close(ls);
    unlink("/tmp/pc-ekran.frame.part");
    unlink("/tmp/pc-ekran.frame");
    printf("FINISHED frames=%u errors=%u bytes=%llu mean_decode_us=%llu stopped=%d\n",frames,errors,
           (unsigned long long)bytes,(unsigned long long)(frames?total_us/frames:0),stopped);
    return errors?1:0;
}

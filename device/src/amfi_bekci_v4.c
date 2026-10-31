/* Amfi gate v4 (24.10.2026, Claude; docs/denetim-2026-10-24-v3.md A10, scripts/628): v3 + saglik denetimi bad_xor'u
 * ilk saglikli okumadaki degere gore yapar (sayac mcu-servis omru boyunca yalniz artar: v3'te tek bozuk XOR amfiyi
 * yeniden baslatmaya kadar susturuyordu). Yeni artis yine hata (cikar, SD betigi yeniden baslatir, yeni taban).
 * MCU hizmeti adi mcu-servis-v6 ya da mcu-servis-v7. Gonderilen komutlar v3 ile ayni.
 * --- v3 ---
 * Amfi gate v3 (22.10.2026, Claude; madde 116, kullanıcı: "bluetooth ile bağlandığımda panelde ses çalsın, bluetooth adını
 * VeLinux KÖLE yap"): v2 + SES KAYNAGI + BT ADI.
 *  - Olcum (22.10 20:5x, kullanıcı telefonla): BT bagli + muzik calarken (IB, MB) v2 amfiyi acti ama kaynak AUX (A7 08 00)
 *    oldugu icin ses yoktu. Melis'in BtMusic sayfasindaki kaynak komutu A7 0b 00 (docs/alinan-melis-mcu §3.5) elle
 *    gonderilince muzik panelden duyuldu; A7 08 00 ile AUX'a donuldu, 2. ekran sesi panelden duyuldu. A7 kalici degil
 *    (docs/ab5301-kalici-ayar-haritasi: kalicilar yalniz A3/A4).
 *  - v3 kaynak karari: panelin kendi sesi (PCM acik) -> AUX; yalniz BT muzigi (baglanti 1, muzik 3) ve son yerel sesten
 *    0.7 sn gecmis -> BtMusic (A7 0b 00); ikisi de yoksa AUX (v2 gibi: ilk kisa panel sesi kaynak degistirmeden calar).
 *  - BT adi: Bluetrum AT#MM adini kalici saklamiyor (fabrika adi SWL-BT; telefonda SWL-BT goruluyordu, AT#MM sonrasi
 *    unut + tara ile VeLinux KOLE). v3 acilista bir kez ve mcu-servis gunlugunde her "BT <t> IS" (BT modulu hazir)
 *    satirinda AT#MM<ad> gonderir; ad /data/bt-ad, yoksa "VeLinux KÖLE". Ad gonderilemezse yalniz gunluge yazar.
 *  mcu-servis'e DOKUNULMAZ (kalp atisi sahibi); ayni FIFO + ayni ACK dogrulamasi. A3/A4 ve ses duzeyi degismez.
 * --- v2 ---
 * Amfi gate v1: existing mcu-servis FIFO ONLY, never opens a UART.
 * Requires confirmed persistent A3=1. AA DATA7 stays1; DATA4 is RAM mute.
 * PCM open/prepared/running or BT MB keeps the gate on; idle500ms mutes.
 * No retry after an uncertain MCU ACK. Existing heartbeat service is untouched.
 */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define KAYNAK_AUX 0x08
#define KAYNAK_BT 0x0b

static volatile sig_atomic_t stopping;
static const char *btpath="/tmp/bt-durum.json";
static const char *logpath="/tmp/mcu-servis-sd.log";
static const char *adpath="/data/bt-ad";
static int active=-1, kaynak=-1, mcu_pid;
static int xor0=-1;	/* v4: ilk saglikli okumadaki bad_xor (-1 = yok) */
static double last_use, last_local=-1e9;
static long is_ofs=-1;	/* v3: mcu-servis gunlugunde IS taramasinin kaldigi yer */
static unsigned long is_say, ad_say;

static double mono(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
static void nap(void) { struct timespec t={0,50000000}; nanosleep(&t,NULL); }
static int readfile(const char *p,char *b,size_t n) {
    int f=open(p,O_RDONLY|O_CLOEXEC); if(f<0) return -1;
    ssize_t z=read(f,b,n-1); close(f); if(z<0) return -1; b[z]=0; return (int)z;
}
static int number(const char *b,const char *key,int fallback) {
    char k[80]; snprintf(k,sizeof k,"\"%s\":",key); const char *p=strstr(b,k);
    return p ? atoi(p+strlen(k)) : fallback;
}
static double realnumber(const char *b,const char *key) {
    char k[80]; snprintf(k,sizeof k,"\"%s\":",key); const char *p=strstr(b,k);
    return p ? atof(p+strlen(k)) : 0;
}
static int health(char *bt,size_t n) {
    if(readfile(btpath,bt,n)<0 || !strstr(bt,"\"running\":true")) return 0;
    int x=number(bt,"bad_xor",-1); if(x<0 || (xor0>=0 && x>xor0)) return 0;	/* v4: yalniz YENI artis hata */
    int p=number(bt,"pid",0); if(p<=1 || (mcu_pid && p!=mcu_pid)) return 0;
    char path[80],comm[80]; snprintf(path,sizeof path,"/proc/%d/comm",p);
    if(readfile(path,comm,sizeof comm)<0 || (strcmp(comm,"mcu-servis-v6\n") && strcmp(comm,"mcu-servis-v7\n"))) return 0;	/* v4: v7 */
    double age=mono()-realnumber(bt,"heartbeat_mono"); if(age<0 || age>3) return 0;
    if(xor0<0) xor0=x;	/* v4: taban ayni mcu-servis surecine (mcu_pid) bagli */
    mcu_pid=p; return 1;
}
/* Acknowledge only a new matching TX followed by AA/A7 ACK00 in the same log.
 * B2 is handled independently by mcu-servis throughout this bounded wait. */
static int command(const char *line,const char *tx,const char *ack) {
    struct stat before,after,fifo;
    if(stat(logpath,&before) || stat("/tmp/mcu-komut",&fifo) || !S_ISFIFO(fifo.st_mode)) return -1;
    int f=open("/tmp/mcu-komut",O_WRONLY|O_NONBLOCK|O_CLOEXEC); if(f<0) return -1;
    size_t n=strlen(line); ssize_t w=write(f,line,n); close(f); if(w!=(ssize_t)n) return -1;
    double end=mono()+1.5;
    while(mono()<end) {
        if(stat(logpath,&after) || before.st_ino!=after.st_ino || after.st_size<before.st_size) return -1;
        if(after.st_size-before.st_size>8191) return -1;
        f=open(logpath,O_RDONLY|O_CLOEXEC); if(f<0) return -1;
        char b[8192]; ssize_t z=pread(f,b,sizeof b-1,before.st_size); close(f);
        if(z>0) { b[z]=0; char *p=strstr(b,tx); if(p && strstr(p,ack)) return 0; }
        nap();
    }
    return -1;
}
static int gate(int on) {
    char bt[1200]; if(!health(bt,sizeof bt)) return -1;
    const char *s=on ? "AA 14 00 07 07 00 00 16 01 14 00\n" : "AA 14 00 07 07 01 00 16 01 14 00\n";
    const char *tx=on ? " 02 80 aa 0a 14 00 07 07 00 00 16 01 14 00 37\n" : " 02 80 aa 0a 14 00 07 07 01 00 16 01 14 00 36\n";
    if(command(s,tx," cevap aa: 00\n")) return -1;
    active=on; printf("AMFI %s ACK00 mono=%.3f A3=1 RAM-only mute\n",on?"ACIK":"KAPALI",mono()); fflush(stdout);
    return 0;
}
/* v3: ses kaynagi (A7, RAM). 0 = ACK00 */
static int kaynak_sec(int k) {
    int rc=k==KAYNAK_BT ? command("A7 0b 00\n"," 02 80 a7 02 0b 00 2e\n"," cevap a7: 00\n")
                        : command("A7 08 00\n"," 02 80 a7 02 08 00 2d\n"," cevap a7: 00\n");
    if(!rc) { kaynak=k; printf("KAYNAK %s ACK00 mono=%.3f\n",k==KAYNAK_BT?"BT (A7 0b 00)":"AUX (A7 08 00)",mono()); fflush(stdout); }
    return rc;
}
/* v3: panelin kendi sesi -> AUX; yalniz BT muzigi (son yerel sesten 0.7 sn sonra) -> BT; hicbiri -> AUX */
static int kaynak_karar(int local,int music,double now,double last_loc) {
    return music && !local && now-last_loc>=0.7 ? KAYNAK_BT : KAYNAK_AUX;
}
/* v3: BT adi: /data/bt-ad (1..40 bayt, kontrol karakteri yok), yoksa varsayilan. FIFO satiri "D4 41 54 23 4d 4d <ad> 0d 0a" */
static int ad_satiri(const char *ad,char *o,size_t n) {
    size_t l=strlen(ad);
    if(!l || l>40) return -1;
    int k=snprintf(o,n,"D4 41 54 23 4d 4d"); if(k<0 || (size_t)k>=n) return -1;
    for(size_t i=0;i<l;i++) {
        unsigned char c=(unsigned char)ad[i];
        if(c<0x20 || c==0x7f) return -1;
        int m=snprintf(o+k,n-k," %02x",c); if(m<0 || (size_t)(k+m)>=n) return -1; k+=m;
    }
    int m=snprintf(o+k,n-k," 0d 0a\n"); if(m<0 || (size_t)(k+m)>=n) return -1;
    return 0;
}
static void bt_ad_gonder(const char *neden) {
    char ad[64]="VeLinux KÖLE", line[400];
    char b[64]; int z=readfile(adpath,b,sizeof b);
    if(z>0) { b[strcspn(b,"\r\n")]=0; if(b[0]) snprintf(ad,sizeof ad,"%s",b); }
    if(ad_satiri(ad,line,sizeof line)) { printf("BT ADI gecersiz, gonderilmedi (%s)\n",neden); fflush(stdout); return; }
    int rc=command(line," 02 80 d4 "," MM\n");
    ad_say++;
    printf("BT ADI %s (%s): %s\n",ad,neden,rc?"cevap yok":"MM ACK"); fflush(stdout);
}
/* v3: tampondaki tam satirlarda "BT <t> IS" sayisi; *tuketilen = son '\n'den sonrasi (yarim satir yeniden okunur) */
static int is_tara(const char *b,size_t n,size_t *tuketilen) {
    int say=0; size_t bas=0; *tuketilen=0;
    for(size_t i=0;i<n;i++) {
        if(b[i]!='\n') continue;
        size_t l=i-bas;
        if(l>6 && !strncmp(b+bas,"BT ",3) && !strncmp(b+i-3," IS",3)) say++;
        bas=i+1; *tuketilen=bas;
    }
    return say;
}
static int is_gordu(void) {
    struct stat s; if(stat(logpath,&s)) return 0;
    if(is_ofs<0) { is_ofs=s.st_size; return 0; }	/* ilk cagri: gunlugun simdiki sonundan */
    if(s.st_size<is_ofs) is_ofs=0;	/* gozetici gunlugu kisalttiysa (64 KiB) basa */
    if(s.st_size==is_ofs) return 0;
    int f=open(logpath,O_RDONLY|O_CLOEXEC); if(f<0) return 0;
    char b[8192]; ssize_t z=pread(f,b,sizeof b,is_ofs); close(f);
    if(z<=0) return 0;
    size_t t; int say=is_tara(b,(size_t)z,&t);
    if(!t) { if(z<(ssize_t)sizeof b) return 0; t=(size_t)z; }	/* yarim satir: sonra; tampondan uzun satir: atla */
    is_ofs+=(long)t;
    is_say+=say;
    return say;
}
static int policy(int state,int local,int bt,int mute,double now,double *last) {
    if(mute) return 0;
    if(local||bt) { *last=now; return 1; }
    return state==1 && now-*last<0.5;
}
static void status(int fault) {
    char t[]="/tmp/amfi-durum.json.yeni";
    FILE *f=fopen(t,"w"); if(!f) return;
    fprintf(f,"{\"schema\":1,\"pid\":%d,\"running\":%s,\"amp_on\":%s,\"fault\":%d,\"mono\":%.3f,\"mcu_pid\":%d,\"kaynak\":\"%s\",\"is\":%lu,\"ad\":%lu,\"surum\":\"amfi-bekci-v4\"}\n",
            getpid(),fault?"false":"true",active==1?"true":"false",fault,mono(),mcu_pid,kaynak==KAYNAK_BT?"bt":kaynak==KAYNAK_AUX?"aux":"?",is_say,ad_say);
    fclose(f); rename(t,"/tmp/amfi-durum.json");
}
static void stop(int s) { (void)s; stopping=1; }
/* v4 test: bu surec "ad" adli mcu-servis gibi; btpath'e saglikli bir durum yazilir, health() sonucu */
static int saglik_dene(const char *ad,int bad_xor) {
    char j[300],b[1200];
    prctl(PR_SET_NAME,ad);
    int k=snprintf(j,sizeof j,"{\"schema\":1,\"running\":true,\"pid\":%d,\"heartbeat_mono\":%.3f,\"bad_xor\":%d,\"service\":\"mcu-servis-v3\"}\n",getpid(),mono()-0.5,bad_xor);	/* %.3f yukari yuvarlamasin */
    int f=open(btpath,O_WRONLY|O_TRUNC|O_CLOEXEC); assert(f>=0 && write(f,j,k)==k); close(f);
    return health(b,sizeof b);
}
static void selftest(void) {
    double t=0;
    assert(policy(-1,0,0,0,1,&t)==0);
    assert(policy(0,1,0,0,2,&t)==1 && t==2);
    assert(policy(1,0,0,0,2.49,&t)==1);
    assert(policy(1,0,0,0,2.5,&t)==0);
    assert(policy(0,0,1,0,3,&t)==1 && t==3);
    assert(policy(1,1,1,1,3.01,&t)==0);
    assert(number("{\"bad_xor\":0}","bad_xor",1)==0);
    assert(number("{}","pid",-1)==-1);
    /* v3 kaynak */
    assert(kaynak_karar(0,0,10,-1e9)==KAYNAK_AUX);
    assert(kaynak_karar(0,1,10,-1e9)==KAYNAK_BT);
    assert(kaynak_karar(1,1,10,10)==KAYNAK_AUX);
    assert(kaynak_karar(0,1,10.5,10)==KAYNAK_AUX);
    assert(kaynak_karar(0,1,10.75,10)==KAYNAK_BT);
    assert(kaynak_karar(1,0,10,10)==KAYNAK_AUX);
    /* v3 ad satiri (cihazda dogrulanan cerceve: AT#MMVeLinux KOLE) */
    char l[400];
    assert(!ad_satiri("VeLinux KÖLE",l,sizeof l));
    assert(!strcmp(l,"D4 41 54 23 4d 4d 56 65 4c 69 6e 75 78 20 4b c3 96 4c 45 0d 0a\n"));
    assert(ad_satiri("",l,sizeof l)==-1);
    assert(ad_satiri("a\tb",l,sizeof l)==-1);
    assert(ad_satiri("12345678901234567890123456789012345678901",l,sizeof l)==-1);
    assert(!ad_satiri("1234567890123456789012345678901234567890",l,sizeof l));
    assert(ad_satiri("VeLinux",l,10)==-1);
    /* v3 IS taramasi: yalniz "BT <t> IS" tam satirlari; yarim satir sonraya kalir */
    size_t tk;
    const char *g="BT 19.222 MM\nBT 20.1 IS\nTX 1 02 80 IS\nB2 ozet IS\nBT 21.5 IS\nBT 22.0 I";
    assert(is_tara(g,strlen(g),&tk)==2 && tk==strlen(g)-strlen("BT 22.0 I"));
    assert(is_tara("BT 1 IA\n",8,&tk)==0 && tk==8);
    assert(is_tara("BT 1 IS",7,&tk)==0 && tk==0);
    /* v3 is_gordu: gercek dosya; ilk cagri sondan, ekleme, yarim satir, kisaltma (gozetici) */
    char tmp[]="/tmp/amfi-v3-testXXXXXX"; int fd=mkstemp(tmp); assert(fd>=0);
    const char *eski=logpath; logpath=tmp; is_ofs=-1; is_say=0;
    assert(write(fd,"BT 1 IS\n",8)==8 && is_gordu()==0);	/* acilistan onceki IS sayilmaz */
    assert(write(fd,"BT 2 MM\nBT 3 IS\nBT 4 I",22)==22 && is_gordu()==1);
    assert(write(fd,"S\n",2)==2 && is_gordu()==1 && is_say==2);
    assert(ftruncate(fd,0)==0 && lseek(fd,0,SEEK_SET)==0 && write(fd,"BT 5 IS\n",8)==8 && is_gordu()==1);
    assert(is_gordu()==0);
    close(fd); unlink(tmp); logpath=eski; is_ofs=-1; is_say=0;
    /* v4 saglik: bad_xor tabani ilk saglikli okumada; yalniz yeni artis hata; v6/v7 adi; baska ad ve eksik sayac hata */
    char ad0[17]={0}; prctl(PR_GET_NAME,ad0);
    char bj[]="/tmp/amfi-v4-btXXXXXX"; int bf=mkstemp(bj); assert(bf>=0); close(bf);
    const char *eskibt=btpath; btpath=bj;
    assert(saglik_dene("mcu-servis-v7",3)==1 && xor0==3 && mcu_pid==getpid());	/* v3: bad_xor 3 -> hep hata */
    assert(saglik_dene("mcu-servis-v7",3)==1);
    assert(saglik_dene("mcu-servis-v7",4)==0 && xor0==3);
    mcu_pid=0; xor0=-1;
    assert(saglik_dene("mcu-servis-v6",0)==1 && xor0==0);
    assert(saglik_dene("mcu-servis-v6",1)==0);
    mcu_pid=0; xor0=-1;
    assert(saglik_dene("mcu-servis-v5",0)==0 && xor0==-1);
    assert(saglik_dene("mcu-servis-v7",-1)==0 && xor0==-1);
    prctl(PR_SET_NAME,ad0); unlink(bj); btpath=eskibt; mcu_pid=0; xor0=-1;
    puts("SELF-TEST PASS: idle/start/tail/BT/master-mute/JSON + v3 kaynak/ad/IS + v4 bad_xor tabani/v6-v7 adi; no device access");
}
int main(int argc,char **argv) {
    if(argc==2 && !strcmp(argv[1],"--self-test")) { selftest(); return 0; }
    if(argc!=2 || strcmp(argv[1],"--a3-kalici-dogrulandi")) { fputs("Requires --a3-kalici-dogrulandi; never provisions A3\n",stderr); return 2; }
    int lock=open("/tmp/amfi-bekci-v1.lock",O_RDWR|O_CREAT|O_CLOEXEC,0600);
    if(lock<0 || flock(lock,LOCK_EX|LOCK_NB)) return 3;
    signal(SIGPIPE,SIG_IGN); signal(SIGTERM,stop); signal(SIGINT,stop); signal(SIGHUP,stop);
    char bt[1200]; double until=mono()+35;
    while(!health(bt,sizeof bt) && mono()<until && !stopping) nap();
    if(stopping || !health(bt,sizeof bt)) { status(1); return 4; }
    if(gate(0)) { status(2); return 5; }
    /* v2: initialize AUX while muted, not during the first short sound. */
    if(kaynak_sec(KAYNAK_AUX)) { status(5); return 8; }
    if(gate(0)) { status(2); return 5; }
    puts("AUX INITIALIZED AT BOOT; AMP MUTED; first short PCM needs no source switch (v3: BT muzigi -> A7 0b 00)"); fflush(stdout);
    is_gordu();	/* v3: gunlugun simdiki sonundan itibaren IS izlenir */
    bt_ad_gonder("acilis");
    double report=0;
    while(!stopping) {
        if(!health(bt,sizeof bt)) { status(3); return 6; }
        char pcm[512]; int z=readfile("/proc/asound/card0/pcm0p/sub0/status",pcm,sizeof pcm);
        if(z<0) { if(active==1) gate(0); status(4); return 7; }
        int local=strncmp(pcm,"closed",6)!=0;
        int music=number(bt,"connection",-1)==1 && number(bt,"music_state",-1)==3;
        int mute=access("/tmp/ses-sessiz",F_OK)==0;
        double now=mono();
        if(local) last_local=now;
        int want=kaynak_karar(local,music,now,last_local);	/* v3 */
        if(want!=kaynak && kaynak_sec(want)) { status(5); return 8; }
        int next=policy(active,local,music,mute,now,&last_use);
        if(next!=active && gate(next)) { status(6); return 9; }
        if(is_gordu()) bt_ad_gonder("BT IS");
        if(now-report>2) { status(0); report=now; }
        nap();
    }
    if(kaynak!=KAYNAK_AUX) kaynak_sec(KAYNAK_AUX);
    int rc=gate(0); status(rc?7:8); close(lock); return rc?10:0;
}

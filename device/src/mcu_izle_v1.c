/* VeLinux passive BT observer. UART is O_RDONLY; there is no transmit path.
 * Only validated D4 event codes are retained. Phone/name/address/track payloads
 * are discarded. Stop this service before running any other mcu reader.
 */
#define _DEFAULT_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t stop;
static struct {
    int connection, music;
    char music_code[3], last_code[3];
    unsigned long frames, bad_xor, events;
    double last_event, last_music;
} state;
static uint8_t rx[512];
static size_t rx_n, line_n;
static char prefix[2];
static int line_overflow;

static double mono(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void reset(void)
{
    memset(&state, 0, sizeof state);
    state.connection = state.music = -1;
    memcpy(state.music_code, "??", 3);
    memcpy(state.last_code, "??", 3);
    rx_n = line_n = 0;
    line_overflow = 0;
}

static void event(const char code[3])
{
    double now = mono();
    memcpy(state.last_code, code, 3);
    state.last_event = now;
    state.events++;
    if (!strcmp(code, "IA")) {
        state.connection = 0;
        state.music = -1;
        state.last_music = 0;
        memcpy(state.music_code, "??", 3);
    } else if (!strcmp(code, "IB")) {
        state.connection = 1;
    } else if (!strcmp(code, "MA") || !strcmp(code, "MC") ||
               !strcmp(code, "MB") || !strcmp(code, "MD") || !strcmp(code, "MY")) {
        state.music = !strcmp(code, "MB") ? 3 :
                      (!strcmp(code, "MA") || !strcmp(code, "MC")) ? 2 : 0;
        memcpy(state.music_code, code, 3);
        state.last_music = now;
    }
    /* Never print the remainder of a BT line. */
    printf("BT %.3f %s\n", now, code);
    fflush(stdout);
}

static void bt_byte(uint8_t b)
{
    if (b == '\r' || b == '\n') {
        if (!line_overflow && line_n >= 2 && prefix[0] >= 'A' && prefix[0] <= 'Z' &&
            prefix[1] >= 'A' && prefix[1] <= 'Z') {
            char code[3] = {prefix[0], prefix[1], 0};
            event(code);
        }
        line_n = 0;
        line_overflow = 0;
    } else {
        if (line_n < 2)
            prefix[line_n] = (char)b;
        if (line_n < 512)
            line_n++;
        else
            line_overflow = 1;
    }
}

static void parse(void)
{
    while (rx_n) {
        if (rx[0] != 2 || (rx_n >= 2 && rx[1] != 0x80 && rx[1] != 0x81)) {
            memmove(rx, rx + 1, --rx_n);
            continue;
        }
        if (rx_n < 5)
            return;
        size_t n = rx[3], total = n + 5;
        if (rx_n < total)
            return;
        uint8_t sum = 0;
        for (size_t i = 1; i < n + 4; i++)
            sum ^= rx[i];
        if (sum != rx[n + 4]) {
            state.bad_xor++;
            line_n = 0;
            line_overflow = 0;
            memmove(rx, rx + 1, --rx_n);
            continue;
        }
        state.frames++;
        if (rx[2] == 0xD4)
            for (size_t i = 0; i < n; i++)
                bt_byte(rx[4 + i]);
        rx_n -= total;
        memmove(rx, rx + total, rx_n);
    }
}

static void feed(const uint8_t *data, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (rx_n == sizeof rx) {
            rx_n = line_n = 0;
            line_overflow = 0;
        }
        rx[rx_n++] = data[i];
        parse();
    }
}

static int format_state(char *out, size_t n, int running)
{
    return snprintf(out, n,
        "{\"schema\":1,\"running\":%s,\"pid\":%ld,\"heartbeat_mono\":%.3f,"
        "\"connection\":%d,\"music_state\":%d,\"music_code\":\"%s\","
        "\"last_code\":\"%s\",\"last_event_mono\":%.3f,\"last_music_mono\":%.3f,"
        "\"rx_frames\":%lu,\"bad_xor\":%lu,\"events\":%lu,\"tx_frames\":0}\n",
        running ? "true" : "false", (long)getpid(), mono(), state.connection,
        state.music, state.music_code, state.last_code, state.last_event,
        state.last_music, state.frames, state.bad_xor, state.events);
}

static int save_state(int running)
{
    char data[768], temp[128];
    int n = format_state(data, sizeof data, running);
    if (n < 0 || (size_t)n >= sizeof data)
        return -1;
    snprintf(temp, sizeof temp, "/tmp/.bt-durum.%ld.tmp", (long)getpid());
    FILE *f = fopen(temp, "w");
    if (!f)
        return -1;
    int ok = fwrite(data, 1, n, f) == (size_t)n;
    if (fclose(f))
        ok = 0;
    if (ok && !rename(temp, "/tmp/bt-durum.json"))
        return 0;
    unlink(temp);
    return -1;
}

/* Build simulated RX frames in memory only. No UART or file writes. */
static void test_frame(const char *s, int corrupt)
{
    uint8_t b[260] = {2, 0x81, 0xD4};
    size_t n = strlen(s);
    assert(n <= 255);
    b[3] = (uint8_t)n;
    memcpy(b + 4, s, n);
    uint8_t x = 0;
    for (size_t i = 1; i < n + 4; i++)
        x ^= b[i];
    b[n + 4] = x ^ (corrupt ? 1 : 0);
    /* Exercise byte-at-a-time fragmentation. */
    for (size_t i = 0; i < n + 5; i++)
        feed(b + i, 1);
}

static int self_test(void)
{
    reset();
    const uint8_t noise[] = {0, 0xff, 2, 0x44};
    feed(noise, sizeof noise);
    test_frame("IB\r\n", 1);
    assert(state.events == 0 && state.bad_xor == 1);
    test_frame("I", 0);
    test_frame("B\r\nADPrivateAddress\r\nSAPrivatePhone\r\n", 0);
    assert(state.connection == 1 && state.events == 3);
    test_frame("MB\r\nMIPrivateTrack\r\n", 0);
    assert(state.music == 3 && !strcmp(state.music_code, "MB"));
    test_frame("MA\r\n", 0);
    assert(state.music == 2);
    char json[768];
    format_state(json, sizeof json, 1);
    assert(!strstr(json, "Private") && strstr(json, "\"tx_frames\":0"));
    test_frame("IA\r\n", 0);
    assert(state.connection == 0 && state.music == -1 && state.last_music == 0);
    puts("SELF-TEST OK: fragmented RX, bad XOR, resync, states, privacy; no UART opened");
    return 0;
}

static void on_signal(int sig)
{
    (void)sig;
    stop = 1;
}

int main(int argc, char **argv)
{
    const char *tty = "/dev/ttyS4";
    double seconds = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--self-test"))
            return self_test();
        if (!strcmp(argv[i], "--seconds") && i + 1 < argc) {
            char *end;
            seconds = strtod(argv[++i], &end);
            if (*end || seconds < 0 || seconds > 86400)
                return 2;
        } else if (!strcmp(argv[i], "--tty") && i + 1 < argc) {
            tty = argv[++i];
        } else {
            fprintf(stderr, "usage: mcu-izle [--seconds 0..86400] [--tty /dev/ttyS4] | --self-test\n");
            return 2;
        }
    }
    int lock = open("/tmp/mcu-izle.lock", O_CREAT | O_RDWR, 0600);
    if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB)) {
        perror("observer lock");
        return 1;
    }
    int fd = open(tty, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror(tty);
        return 1;
    }
    struct termios t;
    if (tcgetattr(fd, &t)) {
        perror("tcgetattr");
        return 1;
    }
    cfmakeraw(&t);
    cfsetispeed(&t, B115200);
    cfsetospeed(&t, B115200);
    t.c_cflag &= ~(CRTSCTS | CSTOPB | PARENB | CSIZE);
    t.c_cflag |= CLOCAL | CREAD | CS8;
    t.c_cc[VMIN] = t.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &t)) {
        perror("tcsetattr");
        return 1;
    }
    /* No tcflush: retain pending RX. O_RDONLY makes UART writes impossible. */
    reset();
    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);
    printf("mcu-izle v1: %s O_RDONLY, 115200; event codes only; TX path absent\n", tty);
    fflush(stdout);
    double started = mono(), saved = -1;
    int result = 0;
    while (!stop && (!seconds || mono() - started < seconds)) {
        if (mono() - saved >= 1) {
            if (save_state(1)) {
                perror("status file");
                result = 1;
                break;
            }
            saved = mono();
        }
        struct pollfd p = {fd, POLLIN, 0};
        int r = poll(&p, 1, 200);
        if (r < 0 && errno != EINTR) {
            perror("poll");
            result = 1;
            break;
        }
        if (r > 0 && (p.revents & (POLLERR | POLLHUP | POLLNVAL))) {
            fputs("UART disconnected\n", stderr);
            result = 1;
            break;
        }
        if (r > 0 && (p.revents & POLLIN)) {
            uint8_t b[256];
            ssize_t n = read(fd, b, sizeof b);
            if (n > 0)
                feed(b, (size_t)n);
            else if (n < 0 && errno != EAGAIN && errno != EINTR) {
                perror("read");
                result = 1;
                break;
            }
        }
    }
    if (save_state(0))
        result = 1;
    close(fd);
    close(lock);
    printf("Finished: frames=%lu bad_xor=%lu events=%lu; TX=0\n", state.frames, state.bad_xor, state.events);
    return result;
}

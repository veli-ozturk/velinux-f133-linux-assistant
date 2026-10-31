/* Passive MCU status consumer: no device control, no UART access. */
#ifndef VELINUX_BT_STATUS_H
#define VELINUX_BT_STATUS_H
#include "cJSON.h"
#include <math.h>
#include <string.h>

struct bt_status {
    int fresh, connection, music_event;
    double music_age;
};

static double bt_number(cJSON *o, const char *key, double fallback)
{
    cJSON *v = cJSON_GetObjectItemCaseSensitive(o, key);
    return cJSON_IsNumber(v) && isfinite(v->valuedouble) ? v->valuedouble : fallback;
}

static struct bt_status bt_status_parse(const char *json, double now)
{
    struct bt_status s = {0, -1, 0, 0};
    cJSON *o = cJSON_Parse(json);
    if (!cJSON_IsObject(o) || !isfinite(now))
        goto done;
    double heartbeat = bt_number(o, "heartbeat_mono", -1);
    if (bt_number(o, "schema", -1) != 1 ||
        !cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "running")) ||
        heartbeat < 0 || now < heartbeat || now - heartbeat >= 5)
        goto done;
    s.fresh = 1;
    double connection = bt_number(o, "connection", -1);
    if (connection == 0 || connection == 1)
        s.connection = (int)connection;
    double last = bt_number(o, "last_music_mono", -1);
    cJSON *code = cJSON_GetObjectItemCaseSensitive(o, "music_code");
    if (last > 0 && last <= now && cJSON_IsString(code) &&
        (!strcmp(code->valuestring, "MA") || !strcmp(code->valuestring, "MB") ||
         !strcmp(code->valuestring, "MC") || !strcmp(code->valuestring, "MD") ||
         !strcmp(code->valuestring, "MY"))) {
        s.music_event = 1;
        s.music_age = now - last;
    }
done:
    cJSON_Delete(o);
    return s;
}
#endif

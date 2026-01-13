// gps_thread.c – GPS reader thread, NMEA parsing, double-buffered fix state

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "gps_sensor.h"

/* =========================================================
 *  Internal double-buffered state
 * ========================================================= */

static struct gps_fix g_fix_buf[2];
static volatile uint32_t g_fix_version;  // increments each publish

/* =========================================================
 *  NMEA checksum validation
 * ========================================================= */

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

/* Validate checksum for a sentence like: $.....*HH */
static bool nmea_checksum_ok(const char *s)
{
    if (!s || s[0] != '$') return false;

    const char *star = strrchr(s, '*');
    if (!star || (star - s) < 1) return false;
    if (!star[1] || !star[2]) return false;

    int h1 = hexval(star[1]);
    int h2 = hexval(star[2]);
    if (h1 < 0 || h2 < 0) return false;

    uint8_t want = (uint8_t)((h1 << 4) | h2);
    uint8_t calc = 0;

    for (const char *p = s + 1; p < star; p++) {
        calc ^= (uint8_t)*p;
    }

    return (calc == want);
}

/* =========================================================
 *  Helpers
 * ========================================================= */

/* Convert ddmm.mmmm or dddmm.mmmm -> decimal degrees (always positive) */
static double nmea_to_degrees(double val)
{
    int deg = (int)(val / 100.0);
    double minutes = val - (deg * 100.0);
    return (double)deg + minutes / 60.0;
}

/* Split NMEA field by ',' in-place */
static char *next_field(char **save)
{
    char *s = *save;
    if (!s || *s == '\0') {
        return NULL;
    }

    char *comma = strchr(s, ',');
    if (comma) {
        *comma = '\0';
        *save = comma + 1;
    } else {
        *save = s + strlen(s);
    }
    return s;
}

/* =========================================================
 *  GGA / RMC parsers
 * ========================================================= */

/* Parse GGA: $GPGGA,time,lat,NS,lon,EW,fix,numsats,hdop,alt,unit,... */
static void gps_parse_gga(const char *line, struct gps_fix *fix)
{
    char buf[160];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    char *p = buf;
    char *field;

    field = next_field(&p);           /* $GPGGA/$GNGGA */
    field = next_field(&p);           /* time   */
    const char *time_str = field;

    field = next_field(&p);           /* lat    */
    const char *lat_str = field;
    field = next_field(&p);           /* N/S    */
    const char *lat_ns  = field;

    field = next_field(&p);           /* lon    */
    const char *lon_str = field;
    field = next_field(&p);           /* E/W    */
    const char *lon_ew  = field;

    field = next_field(&p);           /* fix quality */
    int fix_quality = (field && *field) ? atoi(field) : 0;

    field = next_field(&p);           /* num sats */
    int num_sats = (field && *field) ? atoi(field) : 0;

    field = next_field(&p);           /* hdop (ignored) */
    field = next_field(&p);           /* altitude */
    double alt = (field && *field) ? atof(field) : 0.0;

    fix->fix_quality = (uint8_t)fix_quality;
    fix->num_sats    = num_sats;
    fix->altitude_m  = alt;

    /* Time hhmmss.sss */
    if (time_str && strlen(time_str) >= 6) {
        char tmp[3];
        tmp[2] = '\0';

        tmp[0] = time_str[0];
        tmp[1] = time_str[1];
        fix->hour = (uint8_t)atoi(tmp);

        tmp[0] = time_str[2];
        tmp[1] = time_str[3];
        fix->minute = (uint8_t)atoi(tmp);

        tmp[0] = time_str[4];
        tmp[1] = time_str[5];
        fix->second = (uint8_t)atoi(tmp);
    }

    /* Position: only update coords when we actually have a fix */
    if (fix_quality > 0 && lat_str && *lat_str && lon_str && *lon_str) {

        double lat_raw = atof(lat_str);
        double lon_raw = atof(lon_str);

        double lat_deg_abs = nmea_to_degrees(lat_raw);
        double lon_deg_abs = nmea_to_degrees(lon_raw);

        char ns = (lat_ns && *lat_ns) ? *lat_ns : 'N';
        char ew = (lon_ew && *lon_ew) ? *lon_ew : 'E';

        /* Store absolute + hemisphere for debug/printing */
        fix->latitude_deg  = lat_deg_abs;
        fix->lat_hem       = ns;
        fix->longitude_deg = lon_deg_abs;
        fix->lon_hem       = ew;

        /* Compute signed coords for payload */
        double lat_signed = (ns == 'S') ? -lat_deg_abs : lat_deg_abs;
        double lon_signed = (ew == 'W') ? -lon_deg_abs : lon_deg_abs;

        fix->latitude_e7  = (int32_t)llround(lat_signed * 1e7);
        fix->longitude_e7 = (int32_t)llround(lon_signed * 1e7);

        fix->valid = true;
    } else {
        /* No fix in this GGA (keep last coords, but mark invalid) */
        fix->valid = false;
    }
}

/* Parse RMC mainly to keep time + validity (status A/V) */
static void gps_parse_rmc(const char *line, struct gps_fix *fix)
{
    char buf[160];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    char *p = buf;
    char *field;

    field = next_field(&p);           /* $GPRMC/$GNRMC */
    field = next_field(&p);           /* time */
    const char *time_str = field;

    field = next_field(&p);           /* status A/V */
    const char *status_str = field;

    if (status_str && *status_str == 'A' &&
        time_str && strlen(time_str) >= 6) {

        char tmp[3];
        tmp[2] = '\0';

        tmp[0] = time_str[0];
        tmp[1] = time_str[1];
        fix->hour = (uint8_t)atoi(tmp);

        tmp[0] = time_str[2];
        tmp[1] = time_str[3];
        fix->minute = (uint8_t)atoi(tmp);

        tmp[0] = time_str[4];
        tmp[1] = time_str[5];
        fix->second = (uint8_t)atoi(tmp);
    }
}

/* =========================================================
 *  Publish / snapshot (lock-free double buffer)
 * ========================================================= */

static void gps_state_publish(const struct gps_fix *src)
{
    uint32_t next = g_fix_version + 1U;
    g_fix_buf[next & 1U] = *src;
    g_fix_version = next;
}

void gps_get_fix(struct gps_fix *out)
{
    if (!out) return;

    uint32_t v1, v2;
    do {
        v1 = g_fix_version;
        *out = g_fix_buf[v1 & 1U];
        v2 = g_fix_version;
    } while (v1 != v2);
}

/* =========================================================
 *  GPS reader thread: owns UART reading + parsing
 * ========================================================= */

static void gps_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    /* Init GPS UART here (thread owns UART) */
    while (gps_sensor_init() != 0) {
        printk("GPS thread: gps_sensor_init failed, retrying...\n");
        k_sleep(K_SECONDS(1));
    }

    printk("GPS thread started.\n");

    struct gps_fix fix;
    memset(&fix, 0, sizeof(fix));
    fix.valid        = false;
    fix.fix_quality  = 0;
    fix.lat_hem      = 'N';
    fix.lon_hem      = 'E';
    fix.last_gga[0]  = '\0';
    fix.last_rmc[0]  = '\0';
    fix.last_raw[0]  = '\0';
    fix.latitude_e7  = 0;
    fix.longitude_e7 = 0;

    char line[160];
    size_t idx = 0;
    bool in_sentence = false;

    while (1) {
        uint8_t c;
        int ret = gps_sensor_read_char(&c);

        if (ret == 0) {
            if (c == '\r') {
                continue;
            }

            if (!in_sentence) {
                if (c == '$') {
                    in_sentence = true;
                    idx = 0;
                    line[idx++] = '$';
                }
            } else {
                if (c == '\n') {
                    if (idx > 0) {
                        line[idx] = '\0';

                        /* Save last raw line */
                        strncpy(fix.last_raw, line, sizeof(fix.last_raw));
                        fix.last_raw[sizeof(fix.last_raw) - 1] = '\0';

                        /* Only parse if checksum is OK */
                        bool csum_ok = nmea_checksum_ok(line);

                        if (csum_ok) {
                            if (strncmp(line, "$GPGGA", 6) == 0 ||
                                strncmp(line, "$GNGGA", 6) == 0) {

                                strncpy(fix.last_gga, line, sizeof(fix.last_gga));
                                fix.last_gga[sizeof(fix.last_gga) - 1] = '\0';
                                gps_parse_gga(line, &fix);
                                gps_state_publish(&fix);

                            } else if (strncmp(line, "$GPRMC", 6) == 0 ||
                                       strncmp(line, "$GNRMC", 6) == 0) {

                                strncpy(fix.last_rmc, line, sizeof(fix.last_rmc));
                                fix.last_rmc[sizeof(fix.last_rmc) - 1] = '\0';
                                gps_parse_rmc(line, &fix);
                                gps_state_publish(&fix);
                            }
                        }
                    }

                    in_sentence = false;
                    idx = 0;
                } else {
                    if (idx < sizeof(line) - 1) {
                        line[idx++] = (char)c;
                    } else {
                        in_sentence = false;
                        idx = 0;
                    }
                }
            }
        } else {
            k_msleep(5);
        }
    }
}

/* Auto-start GPS thread at boot */
K_THREAD_DEFINE(gps_thread_id,
                2048,
                gps_thread_fn,
                NULL, NULL, NULL,
                4,
                0,
                0);
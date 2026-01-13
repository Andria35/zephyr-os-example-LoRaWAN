#ifndef GPS_SENSOR_H
#define GPS_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

int gps_sensor_init(void);
int gps_sensor_read_char(uint8_t *out_char);

/*
 * High-level GPS fix state (maintained by gps_thread.c)
 */
struct gps_fix {
    bool   valid;           /* true if we currently have a valid position fix */

    uint8_t fix_quality;    /* from GGA: 0=no fix, 1=GPS fix, 2=DGPS, ... */
    int     num_sats;       /* satellites used in solution */

    /* Human/debug representation */
    double latitude_deg;    /* absolute decimal degrees (0..90) */
    char   lat_hem;         /* 'N' or 'S' */

    double longitude_deg;   /* absolute decimal degrees (0..180) */
    char   lon_hem;         /* 'E' or 'W' */

    double altitude_m;      /* meters */

    uint8_t hour;           /* UTC time from GPS (0–23) */
    uint8_t minute;         /* 0–59 */
    uint8_t second;         /* 0–59 */

    /* Payload-friendly signed coords: degrees * 1e7 */
    int32_t latitude_e7;    /* signed (N positive, S negative) */
    int32_t longitude_e7;   /* signed (E positive, W negative) */

    /* Debug: last parsed sentences */
    char   last_gga[120];
    char   last_rmc[120];
    char   last_raw[120];
};

/* Thread-safe snapshot */
void gps_get_fix(struct gps_fix *out);

/* Optional debug helper (don’t run together with gps thread) */
void gps_debug_raw_reader(void);

#endif /* GPS_SENSOR_H */
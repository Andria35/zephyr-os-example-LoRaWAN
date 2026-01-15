#include <zephyr/device.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include "drivers/humidity_sensor.h"
#include "drivers/leds.h"
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h> 

/* ============================================================
 * =====================  NEW (GPS)  ===========================
 * gps_thread.c auto-starts at boot (K_THREAD_DEFINE),
 * initializes UART inside the GPS thread, parses NMEA,
 * and exposes latest fix via gps_get_fix().
 * ============================================================ */
#include "drivers/gps_sensor.h"
#include "drivers/light_sensor.h"
#include "drivers/soil_sensor.h"

/* ============================================================ */

/* Customize based on network configuration */
#define LORAWAN_DEV_EUI			{ 0x76, 0x39, 0x32, 0x35, 0x59, 0x37, 0x91, 0x94 } // Use your own DEV_EUI
#define LORAWAN_JOIN_EUI		{ 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0xFC, 0x4D }
#define LORAWAN_APP_KEY			{ 0xf3, 0x1c, 0x2e, 0x8b, 0xc6, 0x71, 0x28, 0x1d, 0x51, 0x16, 0xf0, 0x8f, 0xf0, 0xb7, 0x92, 0x8f }

#define DELAY K_MSEC(10000)  /* 30 seconds */
#define MAX_PAYLOAD_SIZE   30
#define NUM_MAX_RETRIES    30

#define DL_LED_PORT 2

#define ADC_RESOLUTION  12
#define ADC_MAX_VALUE   ((1 << ADC_RESOLUTION) - 1) 

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lorawan_class_a);

/* ============================================================
 *  GPS mock fallback
 * ============================================================ */
static void gps_fill_mock(struct gps_fix *fix)
{
    if (!fix) return;

    /* MOCK location: Tokyo (intentionally NOT Spain) */
    fix->valid = true;
    fix->num_sats = 10;

    fix->latitude_deg  = 35.689500;
    fix->lat_hem       = 'N';

    fix->longitude_deg = 139.691700;
    fix->lon_hem       = 'E';

    fix->altitude_m = 40.0;

    fix->hour = 12;
    fix->minute = 0;
    fix->second = 0;

    fix->last_raw[0] = '\0';
    fix->last_gga[0] = '\0';
    fix->last_rmc[0] = '\0';
}

int32_t light_raw_to_pct_x10(int16_t raw)
{
    if (raw < 0) raw = 0;
    if (raw > ADC_MAX_VALUE) raw = ADC_MAX_VALUE;

    /* percentage * 10 (0..1000), with rounding */
    return ((int32_t)raw * 1000 + (ADC_MAX_VALUE / 2)) / ADC_MAX_VALUE;
}

static int32_t soil_mv_to_pct_x10(int32_t mv)
{
    if (mv <= 0) {
        return 0;
    }
    if (mv >= 3300) {
        return 1000; /* 100.0% */
    }
    return (mv * 1000) / 3300;
}

static void trim_ascii(char *s)
{
    if (!s) return;

    /* Trim leading spaces */
    while (*s && isspace((unsigned char)*s)) {
        memmove(s, s + 1, strlen(s));
    }

    /* Trim trailing spaces */
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
}

static bool streq_nocase(const char *a, const char *b)
{
    if (!a || !b) return false;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return false;
        }
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

static void dl_callback(uint8_t port, uint8_t flags, int16_t rssi, int8_t snr,
                        uint8_t len, const uint8_t *hex_data)
{
    LOG_INF("DL: Port %u, Pending %u, RSSI %ddB, SNR %ddBm, TimeUpd %u",
            port, (flags & LORAWAN_DATA_PENDING) != 0, rssi, snr,
            (flags & LORAWAN_TIME_UPDATED) != 0);

    if (!hex_data || len == 0) {
        LOG_WRN("DL: empty payload");
        return;
    }

    LOG_HEXDUMP_INF(hex_data, len, "DL payload:");

    /* Only handle our control port */
    if (port != 2) {
        LOG_WRN("DL: ignoring port %u (expected port 2)", port);
        return;
    }

    /* Copy to a safe, NUL-terminated string */
    char cmd[16];
    size_t copy_len = len;
    if (copy_len >= sizeof(cmd)) {
        copy_len = sizeof(cmd) - 1;
    }
    memcpy(cmd, hex_data, copy_len);
    cmd[copy_len] = '\0';

    trim_ascii(cmd);

    LOG_INF("DL: cmd='%s'", cmd);

    if (streq_nocase(cmd, "OFF")) {
        rgb_set(false, false, false);
        LOG_INF("DL: RGB -> OFF");
    } else if (streq_nocase(cmd, "Green")) {
        rgb_set(false, true, false);
        LOG_INF("DL: RGB -> GREEN");
    } else if (streq_nocase(cmd, "Red")) {
        rgb_set(true, false, false);
        LOG_INF("DL: RGB -> RED");
    } else {
        LOG_WRN("DL: unknown command '%s' (expected OFF/Green/Red)", cmd);
    }
}

static void lorwan_datarate_changed(enum lorawan_datarate dr)
{
	uint8_t unused, max_size;

	lorawan_get_payload_sizes(&unused, &max_size);
	LOG_INF("New Datarate: DR_%d, Max Payload %d", dr, max_size);
}

/* ============================================================
 *  Helper: print payload as one continuous hex string
 * ============================================================ */
static void log_payload_hex_string(const uint8_t *buf, size_t len)
{
    char line[2 * MAX_PAYLOAD_SIZE + 1];
    size_t out = 0;

    if (!buf) return;
    if (len > MAX_PAYLOAD_SIZE) len = MAX_PAYLOAD_SIZE;

    for (size_t i = 0; i < len; i++) {
        if (out + 2 >= sizeof(line)) break;
        snprintk(&line[out], sizeof(line) - out, "%02x", buf[i]);
        out += 2;
    }
    line[out] = '\0';

    LOG_INF("Uplink payload (hex string): %s", line);
}

int main(void)
{
	const struct device *lora_dev;
	struct lorawan_join_config join_cfg;
	uint8_t dev_eui[] = LORAWAN_DEV_EUI;
	uint8_t join_eui[] = LORAWAN_JOIN_EUI;
	uint8_t app_key[] = LORAWAN_APP_KEY;
	int ret;
	uint8_t joining_retries = 0 ;
	
	struct lorawan_downlink_cb downlink_cb = {
		.port = LW_RECV_PORT_ANY,
		.cb = dl_callback
	};

	lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
	if (!device_is_ready(lora_dev)) {
		LOG_ERR("%s: device not ready.", lora_dev->name);
		return 0;
	}

	    /* ---- Init humidity sensor (SI7021) ---- */
    ret = humidity_sensor_init();
    if (ret) {
        LOG_ERR("humidity_sensor_init failed: %d", ret);
        /* continue anyway */
    }

	ret = light_sensor_init();
    if (ret) {
        LOG_ERR("light_sensor_init failed: %d", ret);
        /* continue anyway */
    }

	ret = soil_sensor_init();
	if (ret) {
    	LOG_ERR("soil_sensor_init failed: %d", ret);
    	/* continue anyway */
	}

	ret = leds_init();
    if (ret) LOG_ERR("leds_init failed: %d", ret);

    LOG_INF("GPS thread should auto-start; waiting a moment...");
    k_sleep(K_SECONDS(1));

#if defined(CONFIG_LORAMAC_REGION_EU868)
	/* If more than one region Kconfig is selected, app should set region
	 * before calling lorawan_start()
	 */
	ret = lorawan_set_region(LORAWAN_REGION_EU868);
	if (ret < 0) {
		LOG_ERR("lorawan_set_region failed: %d", ret);
		return 0;
	}
#endif

	ret = lorawan_start();
	if (ret < 0) {
		LOG_ERR("lorawan_start failed: %d", ret);
		return 0;
	}
	
	// change back to false
	lorawan_enable_adr(true); // enable adaptative data rate. not recommended for mobile (non-static position) devices

	lorawan_register_downlink_callback(&downlink_cb);
	lorawan_register_dr_changed_callback(lorwan_datarate_changed);

	join_cfg.mode = LORAWAN_ACT_OTAA;
	join_cfg.dev_eui = dev_eui;
	join_cfg.otaa.join_eui = join_eui;
	join_cfg.otaa.app_key = app_key;
	join_cfg.otaa.nwk_key = app_key;
	
	printf("\r\n DEV_EUI: ");
    for (int i = 0; i < sizeof(dev_eui); ++i) printf("%02x", dev_eui[i]);
    printf("\r\n APP_EUI: ");
    for (int i = 0; i < sizeof(join_eui); ++i) printf("%02x", join_eui[i]);
    printf("\r\n APP_KEY: ");
    for (int i = 0; i < sizeof(app_key); ++i) printf("%02x", app_key[i]);
    printf("\r\n");

	LOG_INF("Waiting 10 seconds to join...");
	k_sleep(K_SECONDS(10));  // Wait 30 seconds


	LOG_INF("Joining network over OTAA");
	while ((ret = lorawan_join(&join_cfg)) < 0) {
		joining_retries++;
		if (joining_retries > NUM_MAX_RETRIES){
			LOG_ERR("Number max of retries (%d/%d) reached, finishing program", joining_retries, NUM_MAX_RETRIES);
			return 0;
		}
		LOG_ERR("Join failed (%d), retrying in 30 seconds, try #%d...", ret, joining_retries);
		k_sleep(K_SECONDS(30));
	}
		
	LOG_INF("Starting to send data...");
	
	static uint16_t counter = 0;		
	uint8_t data[MAX_PAYLOAD_SIZE];
	
while (1) {

    /* ---- Read & print humidity sensor ---- */
    int32_t hum_x100 = 0;
    int32_t temp_x100 = 0;

    ret = humidity_sensor_read(&hum_x100, &temp_x100);
    if (ret < 0) {
        LOG_ERR("humidity_sensor_read failed: %d", ret);
        k_sleep(DELAY);
        continue;
    }

    LOG_INF("TEMP: %d.%02d C | HUM: %d.%02d %%",
            temp_x100 / 100, temp_x100 % 100,
            hum_x100 / 100, hum_x100 % 100);


    /* ============================================================
     * ====================== (LIGHT)  ========================
     * 2) READ LIGHT (ADC) + CONVERT TO PERCENTAGE (*10)
     *    - We store as light_pct_x10 (0..1000)
     * ============================================================ */
    int16_t light_raw = 0;
    int32_t light_mv  = 0;
    int32_t light_pct_x10 = 0;   /* default 0.0% if read fails */

    ret = light_sensor_read(&light_raw, &light_mv);
    if (ret < 0) {
        LOG_ERR("light_sensor_read failed: %d (sending 0%%)", ret);
        light_pct_x10 = 0;
    } else {
        light_pct_x10 = light_raw_to_pct_x10(light_raw);
        LOG_INF("LIGHT: raw=%d mv=%ld -> %ld.%01ld %%",
                light_raw, (long)light_mv,
                (long)(light_pct_x10 / 10), (long)(light_pct_x10 % 10));
    }

    /* ============================================================
     * 3) READ SOIL (ADC) + CONVERT TO PERCENTAGE (*10)
     * ============================================================
     * ======================  NEW (SOIL)  =========================
     * We keep soil_pct_x10 so we can both print and encode it later.
     * ============================================================ */
    int16_t soil_raw = 0;
    int32_t soil_mv  = 0;
    int32_t soil_pct_x10 = 0;   /* default 0.0% if read fails */

    ret = soil_sensor_read(&soil_raw, &soil_mv);
    if (ret < 0) {
        LOG_ERR("soil_sensor_read failed: %d (sending 0%%)", ret);
        soil_pct_x10 = 0;
    } else {
        soil_pct_x10 = soil_mv_to_pct_x10(soil_mv);
        LOG_INF("SOIL: raw=%d mv=%ld -> %ld.%01ld %%",
                (int)soil_raw, (long)soil_mv,
                (long)(soil_pct_x10 / 10), (long)(soil_pct_x10 % 10));
    }

    /* ============================================================
     * =====================  (GPS)  ===========================
     * 2) READ GPS SNAPSHOT (from gps_thread.c) AND PRINT IT
     * ============================================================ */

	 struct gps_fix fix;
	 gps_get_fix(&fix);

	if (!fix.valid) {
    	LOG_WRN("GPS: no fix -> using MOCK data (dev only)");
    	gps_fill_mock(&fix);
	}

	LOG_INF("GPS: sats=%d lat=%.6f %c lon=%.6f %c alt=%.1f",
        	fix.num_sats,
        	fix.latitude_deg, fix.lat_hem,
        	fix.longitude_deg, fix.lon_hem,
        	fix.altitude_m);

    /* ============================================================
     * 5) BUILD PAYLOAD (LE)
     *
     * [temp(int16 LE, x100),
     *  hum(uint16 LE, x100),
     *  light(uint16 LE, pct_x10),
     *  soil(uint16 LE, pct_x10),           <-- NEW
     *  lat(int32 LE, deg*1e6),
     *  lon(int32 LE, deg*1e6),
     *  alt(int16 LE, meters)]
     * ============================================================
     * ======================  NEW (SOIL)  =========================
     * We add soil right after light to keep “environmental” block
     * together: temp/hum/light/soil.
     * ============================================================ */

    /* temp/hum */
    int16_t  t = (int16_t)temp_x100;   /* signed */
    uint16_t h = (uint16_t)hum_x100;   /* unsigned */

    /* light/soil percent*10 */
    uint16_t l = (uint16_t)light_pct_x10;  /* 0..1000 */
    uint16_t s = (uint16_t)soil_pct_x10;   /* 0..1000 */

    /* Convert GPS to signed decimal degrees */
    double lat_signed = fix.latitude_deg;
    if (fix.lat_hem == 'S') lat_signed = -lat_signed;

    double lon_signed = fix.longitude_deg;
    if (fix.lon_hem == 'W') lon_signed = -lon_signed;

    /* Scale to microdegrees (int32) */
    int32_t lat_uDeg = (int32_t)lrint(lat_signed * 1000000.0);
    int32_t lon_uDeg = (int32_t)lrint(lon_signed * 1000000.0);

    /* Altitude meters (int16) */
    int16_t alt_m = (int16_t)lrint(fix.altitude_m);

    /* temp (2) */
    data[0] = (uint8_t)(t & 0xFF);
    data[1] = (uint8_t)((t >> 8) & 0xFF);

    /* hum (2) */
    data[2] = (uint8_t)(h & 0xFF);
    data[3] = (uint8_t)((h >> 8) & 0xFF);

    /* light (2) */
    data[4] = (uint8_t)(l & 0xFF);
    data[5] = (uint8_t)((l >> 8) & 0xFF);

    /* ======================  NEW (SOIL)  =========================
     * soil (2)
     * ============================================================ */
    data[6] = (uint8_t)(s & 0xFF);
    data[7] = (uint8_t)((s >> 8) & 0xFF);
    /* ====================  END NEW (SOIL)  ===================== */

    /* lat (4) */
    data[8]  = (uint8_t)(lat_uDeg & 0xFF);
    data[9]  = (uint8_t)((lat_uDeg >> 8) & 0xFF);
    data[10] = (uint8_t)((lat_uDeg >> 16) & 0xFF);
    data[11] = (uint8_t)((lat_uDeg >> 24) & 0xFF);

    /* lon (4) */
    data[12] = (uint8_t)(lon_uDeg & 0xFF);
    data[13] = (uint8_t)((lon_uDeg >> 8) & 0xFF);
    data[14] = (uint8_t)((lon_uDeg >> 16) & 0xFF);
    data[15] = (uint8_t)((lon_uDeg >> 24) & 0xFF);

    /* alt (2) */
    data[16] = (uint8_t)(alt_m & 0xFF);
    data[17] = (uint8_t)((alt_m >> 8) & 0xFF);

    uint8_t len = 18;

        /* ============================================================
         * 4) PRINT FULL PAYLOAD (LOCAL DEBUG)
         * ============================================================ */
        log_payload_hex_string(data, len);

    /* Print payload for debugging / offline LUA testing */
    LOG_HEXDUMP_INF(data, len, "Uplink payload:");

    /* ---- Send uplink ---- */
    ret = lorawan_send(1, data, len, LORAWAN_MSG_UNCONFIRMED);

    if (ret == -EAGAIN) {
        LOG_ERR("lorawan_send EAGAIN (payload too large for current DR). Continuing...");
        k_sleep(DELAY);
        continue;
    }

    if (ret < 0) {
        LOG_ERR("lorawan_send failed: %d", ret);
        return 0;
    }

    LOG_INF("Data sent!! (temp_x100=%d hum_x100=%d)", temp_x100, hum_x100);

    k_sleep(DELAY);
	}
}

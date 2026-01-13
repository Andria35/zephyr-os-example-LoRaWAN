// gps_sensor.c – low-level UART wrapper for GPS

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>

#include "gps_sensor.h"

#define GPS_UART_NODE DT_NODELABEL(usart1)

static const struct device *gps_uart = NULL;

int gps_sensor_init(void)
{
    gps_uart = DEVICE_DT_GET(GPS_UART_NODE);

    if (!device_is_ready(gps_uart)) {
        printk("gps_sensor: UART (usart1) not ready!\n");
        return -ENODEV;
    }

    printk("gps_sensor: init OK (using usart1 from devicetree)\n");
    return 0;
}

int gps_sensor_read_char(uint8_t *out_char)
{
    if (!gps_uart || !device_is_ready(gps_uart)) {
        return -EAGAIN;
    }

    uint8_t c;
    int ret = uart_poll_in(gps_uart, &c);

    if (ret == 0) {
        if (out_char) {
            *out_char = c;
        }
        return 0;   // got a byte
    }

    return -EAGAIN; // no data
}

/* =========================================================
 * Optional: simple raw-line reader for debugging ONLY
 * (Don’t run this together with gps_thread)
 * ========================================================= */

void gps_debug_raw_reader(void)
{
    int ret = gps_sensor_init();
    if (ret) {
        printk("gps_debug_raw_reader: gps_sensor_init failed: %d\n", ret);
        return;
    }

    printk("GPS debug: printing raw NMEA lines from usart1...\n");

    char line[160];
    size_t idx = 0;
    bool in_sentence = false;

    while (1) {
        uint8_t c;
        int rc = gps_sensor_read_char(&c);

        if (rc == 0) {
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
                        printk("[GPS DEBUG] %s\n", line);
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
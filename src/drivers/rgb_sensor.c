#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>
#include "i2c_helpers.h"
#include "rgb_sensor.h"

/* Under the hood: TCS34725 chip */
static const struct i2c_dt_spec rgb_sensor_i2c =
    I2C_DT_SPEC_GET(DT_NODELABEL(tcs34725));

#define CMD_BIT          0x80
#define REG_ENABLE       0x00
#define REG_CDATAL       0x14

static int write_reg(uint8_t reg, uint8_t value)
{
    return i2c_write_u8_dt(&rgb_sensor_i2c, CMD_BIT | reg, value);
}

int rgb_sensor_init(void)
{
    int ret;

    /* Power on (PON) */
    ret = write_reg(REG_ENABLE, 0x01);
    if (ret < 0) {
        printk("RGB sensor: failed to power on\n");
        return ret;
    }

    k_sleep(K_MSEC(3));

    /* Enable ADC (AEN) */
    ret = write_reg(REG_ENABLE, 0x03);
    if (ret < 0) {
        printk("RGB sensor: failed to enable ADC\n");
        return ret;
    }

    printk("RGB sensor initialized\n");
    return 0;
}

int rgb_sensor_read(uint16_t *clear, uint16_t *red,
                    uint16_t *green, uint16_t *blue)
{
    uint8_t buf[8];
    uint8_t reg = CMD_BIT | REG_CDATAL;

    int ret = i2c_burst_read_dt_checked(&rgb_sensor_i2c, reg, buf, sizeof(buf));
    if (ret < 0) {
        printk("RGB sensor: read failed\n");
        return ret;
    }

    *clear = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    *red   = (uint16_t)buf[2] | ((uint16_t)buf[3] << 8);
    *green = (uint16_t)buf[4] | ((uint16_t)buf[5] << 8);
    *blue  = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);

    return 0;
}
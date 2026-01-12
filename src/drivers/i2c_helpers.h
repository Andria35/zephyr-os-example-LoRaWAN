#ifndef I2C_HELPERS_H
#define I2C_HELPERS_H

#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Write 1 byte to an I2C register, with bus-ready + error logging.
 */
static inline int i2c_write_u8_dt(const struct i2c_dt_spec *spec,
                                  uint8_t reg, uint8_t val)
{
    if (!i2c_is_ready_dt(spec)) {
        printk("I2C write: bus not ready (addr 0x%02X)\n", spec->addr);
        return -ENODEV;
    }

    int ret = i2c_reg_write_byte_dt(spec, reg, val);
    if (ret < 0) {
        printk("I2C write: failed to 0x%02X reg 0x%02X (err %d)\n",
               spec->addr, reg, ret);
    }
    return ret;
}

/**
 * Read 1 byte from an I2C register, with bus-ready + error logging.
 */
static inline int i2c_read_u8_dt(const struct i2c_dt_spec *spec,
                                 uint8_t reg, uint8_t *val)
{
    if (!i2c_is_ready_dt(spec)) {
        printk("I2C read: bus not ready (addr 0x%02X)\n", spec->addr);
        return -ENODEV;
    }

    int ret = i2c_reg_read_byte_dt(spec, reg, val);
    if (ret < 0) {
        printk("I2C read: failed from 0x%02X reg 0x%02X (err %d)\n",
               spec->addr, reg, ret);
    }
    return ret;
}

/**
 * Burst-read sequential registers, with bus-ready + error logging.
 */
static inline int i2c_burst_read_dt_checked(const struct i2c_dt_spec *spec,
                                            uint8_t start_reg,
                                            uint8_t *buf,
                                            size_t len)
{
    if (!i2c_is_ready_dt(spec)) {
        printk("I2C burst: bus not ready (addr 0x%02X)\n", spec->addr);
        return -ENODEV;
    }

    int ret = i2c_burst_read_dt(spec, start_reg, buf, len);
    if (ret < 0) {
        printk("I2C burst: failed from 0x%02X reg 0x%02X (err %d)\n",
               spec->addr, start_reg, ret);
    }
    return ret;
}

#endif /* I2C_HELPERS_H */
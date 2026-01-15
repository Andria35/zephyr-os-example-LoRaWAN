/* light_sensor.c */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

#include "light_sensor.h"

#define ADC_NODE        DT_NODELABEL(adc1)
#define ADC_CHANNEL_ID  5

#define ADC_RESOLUTION  12
#define ADC_MAX_VALUE   ((1 << ADC_RESOLUTION) - 1)
/* Board runs from 3.3 V */
#define ADC_REF_MV      3300

static const struct device *const adc_dev = DEVICE_DT_GET(ADC_NODE);
static int16_t sample_buf;

int light_sensor_init(void)
{
    if (!device_is_ready(adc_dev)) {
        printk("ADC device not ready\n");
        return -ENODEV;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = ADC_CHANNEL_ID,
        .differential     = 0,
    };

    int ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret) {
        printk("adc_channel_setup() failed, err=%d\n", ret);
        return ret;
    }

    return 0;
}

int light_sensor_read(int16_t *raw_out, int32_t *mv_out)
{
    struct adc_sequence seq = {
        .channels    = BIT(ADC_CHANNEL_ID),
        .buffer      = &sample_buf,
        .buffer_size = sizeof(sample_buf),
        .resolution  = ADC_RESOLUTION,
    };

    int ret = adc_read(adc_dev, &seq);
    if (ret) {
        printk("adc_read() failed, err=%d\n", ret);
        return ret;
    }

    int32_t raw = sample_buf;
    if (raw < 0) {
        raw = 0;
    }

    int32_t mv = raw * ADC_REF_MV / ADC_MAX_VALUE;

    if (raw_out) {
        *raw_out = sample_buf;
    }
    if (mv_out) {
        *mv_out = mv;
    }

    return 0;
}
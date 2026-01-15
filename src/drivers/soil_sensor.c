#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

#include "soil_sensor.h"

/* A1 pin on NUCLEO-WL55JC:
 * PB2 = ADC1_IN6
 */
#define ADC_NODE        DT_NODELABEL(adc1)
#define SOIL_ADC_CH     4

#define ADC_RESOLUTION  12
#define ADC_MAX_VALUE   ((1 << ADC_RESOLUTION) - 1)
#define ADC_REF_MV      3300

static const struct device *const adc_dev = DEVICE_DT_GET(ADC_NODE);
static int16_t sample_buf;

int soil_sensor_init(void)
{
    if (!device_is_ready(adc_dev)) {
        printk("ADC not ready for soil sensor\n");
        return -ENODEV;
    }

    struct adc_channel_cfg cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = SOIL_ADC_CH,
        .differential     = 0,
    };

    int ret = adc_channel_setup(adc_dev, &cfg);
    if (ret) {
        printk("Soil adc_channel_setup() failed, err=%d\n", ret);
    }
    return ret;
}

int soil_sensor_read(int16_t *raw_out, int32_t *mv_out)
{
    struct adc_sequence seq = {
        .channels    = BIT(SOIL_ADC_CH),
        .buffer      = &sample_buf,
        .buffer_size = sizeof(sample_buf),
        .resolution  = ADC_RESOLUTION,
    };

    int ret = adc_read(adc_dev, &seq);
    if (ret) {
        printk("Soil adc_read() failed, err=%d\n", ret);
        return ret;
    }

    int32_t raw = sample_buf;
    if (raw < 0) raw = 0;

    int32_t mv = raw * ADC_REF_MV / ADC_MAX_VALUE;

    if (raw_out) *raw_out = raw;
    if (mv_out) *mv_out = mv;

    return 0;
}
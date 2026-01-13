#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "leds.h"
LOG_MODULE_REGISTER(leds, LOG_LEVEL_INF);

/* Aliases from your overlay */
#define LEDR_NODE DT_ALIAS(led0r)
#define LEDG_NODE DT_ALIAS(led0g)
#define LEDB_NODE DT_ALIAS(led0b)

BUILD_ASSERT(DT_NODE_HAS_STATUS(LEDR_NODE, okay), "led0r alias missing");
BUILD_ASSERT(DT_NODE_HAS_STATUS(LEDG_NODE, okay), "led0g alias missing");
BUILD_ASSERT(DT_NODE_HAS_STATUS(LEDB_NODE, okay), "led0b alias missing");

static const struct gpio_dt_spec LED_R = GPIO_DT_SPEC_GET(LEDR_NODE, gpios);
static const struct gpio_dt_spec LED_G = GPIO_DT_SPEC_GET(LEDG_NODE, gpios);
static const struct gpio_dt_spec LED_B = GPIO_DT_SPEC_GET(LEDB_NODE, gpios);

int leds_init(void)
{
    if (!device_is_ready(LED_R.port) ||
        !device_is_ready(LED_G.port) ||
        !device_is_ready(LED_B.port)) return -ENODEV;

    int ret = 0;
    ret |= gpio_pin_configure_dt(&LED_R, GPIO_OUTPUT_INACTIVE);
    ret |= gpio_pin_configure_dt(&LED_G, GPIO_OUTPUT_INACTIVE);
    ret |= gpio_pin_configure_dt(&LED_B, GPIO_OUTPUT_INACTIVE);
    return ret;
}

void rgb_set(bool r, bool g, bool b)
{
    gpio_pin_set_dt(&LED_R, r);
    gpio_pin_set_dt(&LED_G, g);
    gpio_pin_set_dt(&LED_B, b);
}
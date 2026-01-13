#pragma once
#include <zephyr/drivers/gpio.h>
#include <stdbool.h>

int  leds_init(void);
void rgb_set(bool r, bool g, bool b);
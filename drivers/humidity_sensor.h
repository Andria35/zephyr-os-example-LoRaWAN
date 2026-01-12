#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <stdint.h>

int humidity_sensor_init(void);
int humidity_sensor_read(int32_t *humidity_x100, int32_t *temp_x100);

#endif /* HUMIDITY_SENSOR_H */
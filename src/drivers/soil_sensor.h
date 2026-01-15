#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <stdint.h>

int soil_sensor_init(void);
int soil_sensor_read(int16_t *raw_out, int32_t *mv_out);

#endif
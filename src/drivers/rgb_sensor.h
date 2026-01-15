#ifndef RGB_SENSOR_H
#define RGB_SENSOR_H

#include <stdint.h>

int rgb_sensor_init(void);
int rgb_sensor_read(uint16_t *clear, uint16_t *red,
                    uint16_t *green, uint16_t *blue);

#endif /* RGB_SENSOR_H */
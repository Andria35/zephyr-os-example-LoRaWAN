/* light_sensor.h */
#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <stdint.h> 

/* Initialise ADC + channel for the light sensor.
 * Returns 0 on success, negative errno on failure.
 */
int light_sensor_init(void);

/* Take one sample.
 * If non-NULL, *raw_out gets the ADC raw value.
 * If non-NULL, *mv_out  gets the approximate millivolts.
 */
int light_sensor_read(int16_t *raw_out, int32_t *mv_out);

#endif /* LIGHT_SENSOR_H */
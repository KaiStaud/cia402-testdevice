#include <stdint.h>

uint32_t calculate_pwm_frequency(uint32_t usteps, uint32_t steps_per_rev, uint32_t velocity_rpm);
uint32_t calculate_frequency_rise(uint32_t target_frequency, uint32_t target_acceleration, uint32_t start_frequency);
uint32_t control_loop(uint32_t target_position, float kp, float kd, float ki);
/*
Trajectory Generator for trapezoidal ramps.

After setting acceleration, deceleration and velocity
the necessary pwm constraints are re-calculated on the current mircostepping constraint.

Used CanOpen Variables:
0x6070 - Target Position
0x607b - Position range limit
0x607d - Software position limit
0x6081 - Profile velocity
0x6083 - Profile acceleration
0x6084 - Profile deceleration
0x6085 - Quick Stop deceleration

Changelog:
2024-12-10 : Created entity
*/

#include <stdint.h>
#include <profile_generators/trapezoidal_ramp.h>
/*
Private Members

*/

/*
Calculate the pwm frequency to match the motors rpm.
param: Microstepping resolution, steps per revolution, velocity in rpm 
return: constant frequency after acceleration
*/
uint32_t calculate_pwm_frequency(uint32_t usteps, uint32_t steps_per_rev, uint32_t velocity_rpm)
{
uint32_t usteps_per_rev = steps_per_rev * usteps;
uint32_t freq = velocity_rpm * usteps_per_rev;
return freq;
}

/*
Calculate the frequency increment for each timestep ( to reach set frequency )
param: target frequency, start frequency,acceleration
return: pwm-delta
*/
uint32_t calculate_frequency_rise(uint32_t target_frequency, uint32_t target_acceleration, uint32_t start_frequency)
{
uint32_t t_accel = target_frequency / target_acceleration;
uint32_t d_freq = target_frequency - start_frequency;
return d_freq / t_accel; 
}

/**
 * Main position control loop.
 * Ensures the destinated target position is reached.
 * param: target position, PID-Parameters
 * return: actual position
 */
uint32_t control_loop(uint32_t target_position, float kp, float kd, float ki){

}
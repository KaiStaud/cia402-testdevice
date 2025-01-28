#include <stdint.h>
/*
Runs the motor untill a set position is reached. 
The user must set the following parameters to fully configure the trapezoidal ramp profile:

0x6081 - Profile Velocity
0x6083 - Profile acceleration : limited by 0x6080 "max. Motor Speed"
0x6084 - Profile deceleration
0x6085 - Quick Stop deceleration

*/
void set_target_position(uint16_t target_position);
void update_actual_position(uint16_t actual_position);
void enable_operation();
void control_loop();
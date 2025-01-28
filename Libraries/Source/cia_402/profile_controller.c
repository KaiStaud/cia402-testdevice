/**
 * 2024-12-09
 */
#include <stdint.h>

enum control_mode{
    mode_none = 0,
    mode_profile_position = 1,
    mode_profile_velocity = 3,
    mode_homing = 6,
};

/**
 *  Change to drive mode
 */
uint16_t request_mode_of_operation(uint16_t op_mode )
{
/* Check if the drive mode is implemented firmware*/
if(op_mode > 10)
{
    return 0;
}
else{
    return op_mode;
}
}
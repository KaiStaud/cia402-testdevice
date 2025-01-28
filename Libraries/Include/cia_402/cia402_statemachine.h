#include <stdint.h>

enum cia402_states
{
not_ready_to_switch_on = 0,
switch_on_disabled = 1,
ready_to_switch_on = 2, 
switched_on =3 ,
operation_enabled =4,
};

enum cia402_transition
{
/* Move up the statemachine*/
transition_to_switch_on_disabled = 1,
transition_to_switch_on = 6,
transition_to_switched_on = 7,
transition_to_operation_enabled = 15,

/* Move down the statemachine */
transition_disable_operation = 5,
transition_switch_off = 6,
transition_disable = 7,

/* Fault handling */
transition_fault_reset = 15,
};

/**
 * Run the state machine 
 * param : controlword ( 6040h )
 * return : statusword ( 6041h )
 */
uint16_t process_controlword(uint16_t controlword);
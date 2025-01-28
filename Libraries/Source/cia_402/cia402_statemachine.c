/**
 * 2024-12-09
 */
#include <stdint.h>
#include <stdbool.h>
#include <cia_402/cia402_statemachine.h>
#define DEBUG 1

#define Mask_Shutdown (1<<1 | 1<<2)
#define Mask_Switch_On (1<<0 | 1<<1 | 1<<2)
#define Mask_Enable_Operation (1<<0|1<<1 | 1<<2 | 1<<3 | 1<<4)
#define Mask_Disable_Voltage (1)

uint16_t boot_statemachine()
{
return switch_on_disabled;
}

/**
 * Check if bit is set
 * param: input word 
 */
bool _is_set(uint16_t controlword,uint16_t mask)
{
    return true;
}
/**
 * Run the state machine 
 * param : controlword ( 6040h )
 * return : statusword ( 6041h )
 */
uint16_t process_controlword(uint16_t controlword)
{
static uint16_t intern_state= switch_on_disabled;

switch(intern_state)
{
    case not_ready_to_switch_on:
    break;

    case switch_on_disabled:
        if(controlword ==  0x6)
        {
            intern_state = ready_to_switch_on;
        }
    break;

    case ready_to_switch_on:
        if(controlword == 0x7)
        {
            intern_state = switched_on;
        }
        if(controlword == 0x2)
        {
        intern_state = switch_on_disabled;
        }
    break;

    case switched_on:
        if(controlword == 0xF)
        {
            intern_state = operation_enabled;
        }
        if(controlword == 0x6)
        {
        intern_state = ready_to_switch_on;
        }
        if(controlword == 0x2)
        {
        intern_state = switch_on_disabled;
        }
    break;

    case operation_enabled:
        if(controlword == 0x7)
        {
            intern_state = switched_on;
        }
        if(controlword == 0x6)
        {
        intern_state = ready_to_switch_on;
        }
        if(controlword == 0x2)
        {
        intern_state = switch_on_disabled;
        }
    break;
}

return intern_state;
}

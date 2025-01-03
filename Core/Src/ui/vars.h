#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_VAR_SET_POSITION = 0,
    FLOW_GLOBAL_VARIABLE_VAR_ACT_POSITION = 1,
    FLOW_GLOBAL_VARIABLE_VAR_ACT_ACCELERATION = 2,
    FLOW_GLOBAL_VARIABLE_VAR_SYSTEMSTATE = 3,
    FLOW_GLOBAL_VARIABLE_VAR_SYSTEM_ERROR = 4
};

// Native global variables

extern int32_t get_var_var_set_position();
extern void set_var_var_set_position(int32_t value);
extern int32_t get_var_var_act_position();
extern void set_var_var_act_position(int32_t value);
extern int32_t get_var_var_act_acceleration();
extern void set_var_var_act_acceleration(int32_t value);
extern const char *get_var_var_systemstate();
extern void set_var_var_systemstate(const char *value);
extern int32_t get_var_var_system_error();
extern void set_var_var_system_error(int32_t value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/
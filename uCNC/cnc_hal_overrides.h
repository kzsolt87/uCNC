#ifndef CNC_HAL_OVERRIDES_H
#define CNC_HAL_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "cnc_hal_reset.h"
#define S_CURVE_ACCELERATION_LEVEL 0
#define ENABLE_EXTRA_SYSTEM_CMDS
// #define RAM_ONLY_SETTINGS
#define DISABLE_ALL_CONTROLS
#define LIMIT_X_PULLUP_ENABLE
#define LIMIT_Y_PULLUP_ENABLE
#define LIMIT_Z_PULLUP_ENABLE
#define DISABLE_PROBE
#define TOOL1 spindle_pwm
#define SPINDLE_PWM PWM0
#define SPINDLE_PWM_DIR DOUT0
#define ENCODERS 0
//Custom configurations
// disable some features to make the code smaller
// #define DISABLE_SETTINGS_MODULES
// #define DISABLE_MULTISTREAM_SERIAL
#define DISABLE_RTC_CODE

#ifdef __cplusplus
}
#endif
#endif

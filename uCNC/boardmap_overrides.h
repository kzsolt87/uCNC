#ifndef BOADMAP_OVERRIDES_H
#define BOADMAP_OVERRIDES_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "boardmap_reset.h"
#define MCU MCU_AVR
#define KINEMATIC KINEMATIC_CARTESIAN
#define AXIS_COUNT 3
#define TOOL_COUNT 1
#define BAUDRATE 9600
#define BOARD BOARD_UNO
#define BOARD_NAME "Arduino UNO"
#define UART_PORT 0
#define ITP_TIMER 1
#define RTC_TIMER 0
#define ONESHOT_TIMER 2
#define STEP0_BIT 7
#define STEP0_PORT D
#define STEP1_BIT 6
#define STEP1_PORT C
#define STEP2_BIT 3
#define STEP2_PORT B
#define DIR0_BIT 5
#define DIR0_PORT C
#define DIR1_BIT 7
#define DIR1_PORT C
#define DIR2_BIT 2
#define DIR2_PORT B
#define STEP0_EN_BIT 6
#define STEP0_EN_PORT D
#define STEP1_EN_BIT 6
#define STEP1_EN_PORT D
#define STEP2_EN_BIT 5
#define STEP2_EN_PORT A
#define LIMIT_X_BIT 2
#define LIMIT_X_PORT C
#define LIMIT_X_PULLUP
#define LIMIT_X_ISR 0
#define LIMIT_Y_BIT 3
#define LIMIT_Y_PORT C
#define LIMIT_Y_PULLUP
#define LIMIT_Y_ISR 0
#define LIMIT_Z_BIT 4
#define LIMIT_Z_PORT C
#define LIMIT_Z_PULLUP
#define LIMIT_Z_ISR 0
#define PROBE_BIT 3
#define PROBE_PORT A
#define PROBE_ISR 1
#define ESTOP_ISR 1
#define FHOLD_ISR 1
#define CS_RES_ISR 1
#define TX_BIT 1
#define TX_PORT D
#define RX_BIT 0
#define RX_PORT D
#define IC74HC595_COUNT 0
//Custom configurations
#define PCINT0_PORT B
#define PCINT1_PORT C
#define PCINT2_PORT D
#define PLANNER_BUFFER_SIZE 12
#undef STEP_ISR_SKIP_MAIN
#undef STEP_ISR_SKIP_IDLE

#ifdef __cplusplus
}
#endif
#endif

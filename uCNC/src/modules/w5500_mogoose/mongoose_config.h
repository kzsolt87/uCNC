#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

#include "../../cnc.h"
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define MG_ARCH MG_ARCH_CUSTOM
// #define MG_ENABLE_SOCKET 0
#define MG_ENABLE_TCPIP 1
#define MG_ENABLE_DRIVER_W5500 1
#define MG_ENABLE_TCPIP_DRIVER_INIT 0
#define MG_ENABLE_CUSTOM_MILLIS 1
#define mkdir(a, b) (-1)
#define MG_ENABLE_LOG 0
// #define MG_ENABLE_CUSTOM_RANDOM 1

#endif
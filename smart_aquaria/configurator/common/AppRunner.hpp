#pragma once

#include "Config.hpp"
#include "IGPIO.h"
#include "ISPI.h"
#include "IUART.h"

// Wires up all application objects and runs the main loop.
// HAL objects are created by the platform-specific main and passed in.
// spi/dc/rst may be nullptr when displayType != "st7789".
int runApp(const AppConfig& cfg,
           IGPIO& buzzerGpio, IGPIO& heaterGpio,
           IUART& tempUart,   IUART& doUart,
           ISPI*  spi,        IGPIO* dc, IGPIO* rst);

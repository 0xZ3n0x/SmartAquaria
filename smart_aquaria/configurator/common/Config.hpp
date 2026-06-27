#pragma once

#include <cstdint>
#include <string>

struct AppConfig
{
    std::string  tempPort, doPort, logFile;
    int          baudRate{};
    float        timeout{};
    std::string  displayLog;
    std::string  displayType;

    // GPIO — Linux uses chip + line, VxWorks uses pin directly
    std::string  gpioChip;
    unsigned int buzzerLine{}, heaterLine{};

    // Thresholds
    float        tempMin{}, tempMax{}, tempCritLo{}, tempCritHi{};
    float        doWarning{}, doCritical{};

    // Timers
    uint32_t     sampleIntervalMs{}, feedIntervalMs{};

    // SPI / display — Linux uses device path, VxWorks uses ctrl index + chip select
    std::string  spiDev;
    uint32_t     spiSpeedHz{};
    uint32_t     spiCtrlIndex{};
    uint8_t      spiChipSelect{};
    unsigned int dcLine{}, rstLine{};
};

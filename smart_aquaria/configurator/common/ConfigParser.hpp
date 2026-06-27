#pragma once

#include <toml++/toml.hpp>

#include "Config.hpp"

inline AppConfig parseConfig(const toml::table& cfg)
{
    AppConfig c;
    c.tempPort   = cfg["serial"]["ports"]["temp"]["main"].value<std::string>().value();
    c.doPort     = cfg["serial"]["ports"]["do"]["main"].value<std::string>().value();
    c.baudRate   = cfg["serial"]["baudrate"].value<int>().value();
    c.timeout    = static_cast<float>(cfg["serial"]["timeout"].value<double>().value());
    c.logFile    = cfg["log"]["main"].value<std::string>().value();
    c.displayLog = cfg["log"]["display"].value<std::string>().value_or("");
    c.gpioChip   = cfg["gpio"]["chip"].value<std::string>().value_or("");
    c.buzzerLine = static_cast<unsigned int>(cfg["gpio"]["buzzer"]["line"].value<int>().value());
    c.heaterLine = static_cast<unsigned int>(cfg["gpio"]["heater"]["line"].value<int>().value());

    c.tempMin    = static_cast<float>(cfg["thresholds"]["temp"]["min"].value<double>().value());
    c.tempMax    = static_cast<float>(cfg["thresholds"]["temp"]["max"].value<double>().value());
    c.tempCritLo = static_cast<float>(cfg["thresholds"]["temp"]["critical_lo"].value<double>().value());
    c.tempCritHi = static_cast<float>(cfg["thresholds"]["temp"]["critical_hi"].value<double>().value());
    c.doWarning  = static_cast<float>(cfg["thresholds"]["do"]["warning"].value<double>().value());
    c.doCritical = static_cast<float>(cfg["thresholds"]["do"]["critical"].value<double>().value());

    c.sampleIntervalMs =
        static_cast<uint32_t>(cfg["intervals"]["sampling_seconds"].value<double>().value() * 1000.0);
    c.feedIntervalMs =
        static_cast<uint32_t>(cfg["intervals"]["feeding_seconds"].value<int>().value() * 1000);

    c.displayType = cfg["display"]["type"].value<std::string>().value_or("terminal");

    if (c.displayType == "st7789")
    {
        c.spiSpeedHz    = static_cast<uint32_t>(cfg["spi"]["speed_hz"].value<int>().value());
        c.dcLine        = static_cast<unsigned int>(cfg["gpio"]["display"]["dc_line"].value<int>().value());
        c.rstLine       = static_cast<unsigned int>(cfg["gpio"]["display"]["rst_line"].value<int>().value());
        c.spiDev        = cfg["spi"]["device"].value<std::string>().value_or("");
        c.spiCtrlIndex  = static_cast<uint32_t>(cfg["spi"]["ctrl_index"].value<int>().value_or(0));
        c.spiChipSelect = static_cast<uint8_t>(cfg["spi"]["chip_select"].value<int>().value_or(0));
    }

    return c;
}

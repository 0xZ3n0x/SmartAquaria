#include <iostream>
#include <memory>

#include <toml++/toml.hpp>

#include "AppRunner.hpp"
#include "ConfigParser.hpp"
#include "GPIO.h"
#include "SPI.h"
#include "UART.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "[fatal] usage: " << argv[0] << " <config_path>" << std::endl;
        return 1;
    }

    try
    {
        const AppConfig cfg = parseConfig(toml::parse_file(argv[1]));

        UART tempUart(cfg.tempPort, cfg.baudRate, cfg.timeout);
        UART doUart(cfg.doPort,   cfg.baudRate, cfg.timeout);

        GPIO buzzerGpio(cfg.gpioChip, cfg.buzzerLine);
        GPIO heaterGpio(cfg.gpioChip, cfg.heaterLine);

        std::unique_ptr<SPI>  spi;
        std::unique_ptr<GPIO> dc;
        std::unique_ptr<GPIO> rst;

        if (cfg.displayType == "st7789")
        {
            spi = std::make_unique<SPI>(cfg.spiDev, cfg.spiSpeedHz, /*mode=*/3);
            dc  = std::make_unique<GPIO>(cfg.gpioChip, cfg.dcLine);
            rst = std::make_unique<GPIO>(cfg.gpioChip, cfg.rstLine);
        }

        return runApp(cfg,
                      buzzerGpio, heaterGpio,
                      tempUart,   doUart,
                      spi.get(),  dc.get(), rst.get());
    }
    catch (const std::exception& e)
    {
        std::cerr << "[fatal] " << e.what() << std::endl;
    }

    return 1;
}

#include <iostream>
#include <memory>

#include <toml++/toml.hpp>

#include "AppRunner.hpp"
#include "ConfigParser.hpp"
#include "GPIO.h"
#include "SPI.h"
#include "UART.h"

static constexpr const char* kDefaultConfigPath = "./config_rpi4_vxworks_dkm.toml";

int main(int argc, char* argv[])
{
    const char* configPath = (argc >= 2) ? argv[1] : kDefaultConfigPath;

    try
    {
        const AppConfig cfg = parseConfig(toml::parse_file(configPath));

        UART tempUart(cfg.tempPort, cfg.baudRate, cfg.timeout);
        UART doUart(cfg.doPort,   cfg.baudRate, cfg.timeout);

        GPIO buzzerGpio(cfg.buzzerLine);
        GPIO heaterGpio(cfg.heaterLine);

        std::unique_ptr<SPI>  spi;
        std::unique_ptr<GPIO> dc;
        std::unique_ptr<GPIO> rst;

        if (cfg.displayType == "st7789")
        {
            spi = std::make_unique<SPI>(cfg.spiCtrlIndex, cfg.spiSpeedHz, cfg.spiChipSelect, /*mode=*/3);
            dc  = std::make_unique<GPIO>(cfg.dcLine);
            rst = std::make_unique<GPIO>(cfg.rstLine);
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

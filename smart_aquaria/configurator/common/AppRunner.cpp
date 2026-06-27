#include "AppRunner.hpp"

#include <csignal>
#include <iostream>
#include <memory>

#include <atomic>
#include "Clock.h"
#include "EventQueue.h"
#include "Logger.h"
#include "TimerService.h"
#include "TerminalDisplayDevice.h"
#include "adapter/actuator/ActuatorAdapter.h"
#include "adapter/display/DisplayAdapter.h"
#include "adapter/sensor/SensorAdapter.h"
#include "app/SmartAquaria.h"
#include "device/actuator/buzzer/BuzzerDevice.h"
#include "device/actuator/heater/HeaterDevice.h"
#include "device/display/st7789/ST7789DisplayDevice.h"
#include "device/sensor/do/EzoDOSensorDevice.h"
#include "device/sensor/temp/EzoRTDSensorDevice.h"
#include "domain/Thresholds.hpp"
#include "renderer/DisplayRenderer.h"

namespace
{
SmartAquaria* g_app = nullptr;
} // namespace

extern "C" void signalHandler([[maybe_unused]] int signum)
{
    if (nullptr != g_app)
    {
        g_app->stop();
    }
}

int runApp(const AppConfig& cfg,
           IGPIO& buzzerGpio, IGPIO& heaterGpio,
           IUART& tempUart,   IUART& doUart,
           ISPI*  spi,        IGPIO* dc, IGPIO* rst)
{
    try
    {
        auto logger = std::make_unique<Logger>(cfg.logFile.c_str());

        EzoRTDSensorDevice tempSensor(tempUart);
        EzoDOSensorDevice  doSensor(doUart);

        BuzzerDevice buzzer(buzzerGpio);
        HeaterDevice heater(heaterGpio);

        SensorAdapter   sensorAdapter(tempSensor, doSensor);
        ActuatorAdapter actuatorAdapter(buzzer, heater);

        auto clock = std::make_unique<Clock>();

        std::unique_ptr<IDisplayDevice> displayDevice;
        if (spi != nullptr && dc != nullptr && rst != nullptr)
        {
            logger->log("init", "initialising ST7789 display");
            displayDevice = std::make_unique<ST7789DisplayDevice>(*spi, *dc, *rst, *clock);
            logger->log("init", "ST7789 display ready");
        }
        else
        {
            logger->log("init", ("using terminal display: " + cfg.displayLog).c_str());
            displayDevice = std::make_unique<TerminalDisplayDevice>(cfg.displayLog.c_str());
        }

        std::unique_ptr<DisplayAdapter> displayAdapter;
        if (displayDevice != nullptr)
        {
            displayAdapter = std::make_unique<DisplayAdapter>(
                std::make_unique<DisplayRenderer>(),
                std::move(displayDevice));
        }
            

        const Thresholds thresholds(cfg.tempMin, cfg.tempMax, cfg.tempCritLo, cfg.tempCritHi,
                                    cfg.doWarning, cfg.doCritical);

        EventQueue   queue;
        std::atomic<bool> shutdown{false};
        SmartAquaria app(sensorAdapter, actuatorAdapter, thresholds, queue, *clock, shutdown);
        logger->log("init", "startup complete — running");
        app.setLogger(std::move(logger));
        app.setDisplay(std::move(displayAdapter));
        app.setFeedInterval(cfg.feedIntervalMs);

        g_app = &app;
        std::signal(SIGINT,  signalHandler);
        std::signal(SIGTERM, signalHandler);

        TimerService sampleTimer;
        sampleTimer.setInterval(cfg.sampleIntervalMs);
        sampleTimer.run([&app]() { app.push(SmartAquaria::Event::SampleTimerElapsed); });
        app.run();

        sampleTimer.stop();
        buzzer.buzz(false);
        heater.heat(false);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[fatal] " << e.what() << std::endl;
    }

    return 1;
}

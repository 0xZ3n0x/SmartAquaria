#include <atomic>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ILogger.h"
#include "adapter/actuator/ActuatorAdapter.h"
#include "adapter/sensor/SensorAdapter.h"
#include "app/SmartAquaria.h"
#include "device/actuator/buzzer/BuzzerDevice.h"
#include "device/actuator/heater/HeaterDevice.h"
#include "device/sensor/do/EzoDOSensorDevice.h"
#include "device/sensor/temp/EzoRTDSensorDevice.h"
#include "domain/IActuator.h"
#include "domain/ISensor.h"
#include "domain/Thresholds.hpp"

// Linux HAL implementations used in tests
#include <atomic>
#include "Clock.h"
#include "EventQueue.h"
#include "IGPIO.h"
#include "IUART.h"
#include "TimerService.h"

// ---------------------------------------------------------------------------
// Mocks
// ---------------------------------------------------------------------------

class MockLogger : public ILogger
{
  public:
    MOCK_METHOD(bool, log, (const char* prefix, const char* message), (override));
};

class MockUART : public IUART
{
  public:
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(int, write, (const char* data, size_t len), (override));
    MOCK_METHOD(int, read, (char* buffer, size_t len), (override));
    MOCK_METHOD(void, flush, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
};

class MockGPIO : public IGPIO
{
  public:
    MOCK_METHOD(bool, write, (bool active), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, close, (), (override));
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Build a SmartAquaria instance together with its infrastructure objects.
/// All objects are owned by the test fixture and must outlive `app`.
struct AppFixture
{
    Clock        clock;
    EventQueue   queue;
    TimerService sampleTimer;

    // Constructed after the infrastructure objects above
    SmartAquaria* app{nullptr};
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST(SmartAquariaTest, RunLogsStateTransitionToNormal)
{
    MockUART mockUart;
    EXPECT_CALL(mockUart, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockUart, write(testing::_, testing::_)).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(mockUart, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "25\n";
            if (*pos >= response.size()) { *pos = 0; return 0; }
            buf[0] = response[(*pos)++];
            return 1;
        });

    MockUART mockDoUart;
    EXPECT_CALL(mockDoUart, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockDoUart, write(testing::_, testing::_)).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(mockDoUart, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "7.00\n";
            if (*pos >= response.size()) { *pos = 0; return 0; }
            buf[0] = response[(*pos)++];
            return 1;
        });

    MockGPIO mockGpio;
    MockGPIO mockHeaterGpio;
    EXPECT_CALL(mockGpio, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockGpio, write(testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockHeaterGpio, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockHeaterGpio, write(testing::_)).WillRepeatedly(testing::Return(true));

    EzoRTDSensorDevice tempDev(mockUart);
    EzoDOSensorDevice  doDev(mockDoUart);
    BuzzerDevice       buzzerDev(mockGpio);
    HeaterDevice       heaterDev(mockHeaterGpio);

    SensorAdapter   sensorAdapter(tempDev, doDev);
    ActuatorAdapter actuatorAdapter(buzzerDev, heaterDev);
    Thresholds      thresholds(20, 30, 16, 34, 6.0F, 5.0F);

    Clock      clock;
    EventQueue queue;
    std::atomic<bool> shutdown{false};
    SmartAquaria     app(sensorAdapter, actuatorAdapter, thresholds, queue, clock, shutdown);

    TimerService sampleTimer;
    sampleTimer.setInterval(200);
    sampleTimer.run([&app]() { app.push(SmartAquaria::Event::SampleTimerElapsed); });

    std::thread t([&app]() { app.run(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    app.stop();
    t.join();
    sampleTimer.stop();
}

TEST(SmartAquariaTest, RunTransitionsToAlarmOnCriticalTemp)
{
    MockUART mockUart;
    EXPECT_CALL(mockUart, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockUart, write(testing::_, testing::_)).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(mockUart, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "35\n";
            if (*pos >= response.size()) { *pos = 0; return 0; }
            buf[0] = response[(*pos)++];
            return 1;
        });

    MockUART mockDoUart;
    EXPECT_CALL(mockDoUart, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockDoUart, write(testing::_, testing::_)).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(mockDoUart, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "7.00\n";
            if (*pos >= response.size()) { *pos = 0; return 0; }
            buf[0] = response[(*pos)++];
            return 1;
        });

    MockGPIO mockGpio;
    MockGPIO mockHeaterGpio;
    EXPECT_CALL(mockGpio, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockGpio, write(testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockHeaterGpio, isOpen()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mockHeaterGpio, write(testing::_)).WillRepeatedly(testing::Return(true));

    EzoRTDSensorDevice tempDev(mockUart);
    EzoDOSensorDevice  doDev(mockDoUart);
    BuzzerDevice       buzzerDev(mockGpio);
    HeaterDevice       heaterDev(mockHeaterGpio);

    SensorAdapter   sensorAdapter(tempDev, doDev);
    ActuatorAdapter actuatorAdapter(buzzerDev, heaterDev);
    Thresholds      thresholds(20, 30, 16, 34, 6.0F, 5.0F);

    Clock      clock;
    EventQueue queue;
    std::atomic<bool> shutdown{false};
    SmartAquaria     app(sensorAdapter, actuatorAdapter, thresholds, queue, clock, shutdown);

    TimerService sampleTimer;
    sampleTimer.setInterval(50);
    sampleTimer.run([&app]() { app.push(SmartAquaria::Event::SampleTimerElapsed); });

    std::thread t([&app]() { app.run(); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    app.stop();
    t.join();
    sampleTimer.stop();
}

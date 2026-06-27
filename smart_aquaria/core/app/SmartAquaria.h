#pragma once

#include <cstdint>

#include <atomic>
#include <memory>
#include <optional>

#include "IClock.h"
#include "IEventQueue.h"
#include "domain/AquariaState.h"
#include "domain/Thresholds.hpp"

class IActuator;
class IDisplay;
class ILogger;
class ISensor;
struct Readings;

class SmartAquaria final
{
  public:
    enum class Event : int
    {
        SampleTimerElapsed
    };

    static constexpr int kHysteresisUp   = 3; // consecutive worse readings required to worsen state
    static constexpr int kFaultThreshold = 5; // consecutive failed reads required to enter SensorFault

    SmartAquaria(ISensor& sensor, IActuator& actuator, const Thresholds& thresholds, IEventQueue& queue,
                 IClock& clock, std::atomic<bool>& shutdown);
    ~SmartAquaria();

    SmartAquaria(const SmartAquaria&)            = delete;
    SmartAquaria& operator=(const SmartAquaria&) = delete;
    SmartAquaria(SmartAquaria&&)                 = delete;
    SmartAquaria& operator=(SmartAquaria&&)      = delete;

    void run();
    void stop();

    void setLogger(std::unique_ptr<ILogger> logger);
    void setDisplay(std::unique_ptr<IDisplay> display);

    void setFeedInterval(uint32_t interval_ms);
    void push(Event event);
    void dispatch(Event event);

    [[nodiscard]] AquariaState currentState() const noexcept;

  private:
    struct SensorData
    {
        std::optional<float> temp;
        std::optional<float> do_val;
    };

    void transit(AquariaState next);
    void handleSample();
    void handleFeed();

    [[nodiscard]] SensorData   readSensors();
    void                       updateHeater(float temp);
    void                       updateFSM(float temp, float doVal);
    void                       checkFeed();
    [[nodiscard]] AquariaState computeTarget(float temp, float doVal) const noexcept;

    void     callDisplay(const SensorData& data) const;
    Readings buildReadings(const SensorData& data) const;
    uint32_t feedTimeRemainingSeconds() const;
    void     log(const char* prefix, const char* message) const;

    // Mandatory dependencies
    ISensor&           m_sensor;
    IActuator&         m_actuator;
    IEventQueue&       m_queue;
    IClock&            m_clock;
    std::atomic<bool>& m_shutdown;

    // Configuration
    Thresholds m_thresholds;
    uint32_t   m_feed_interval_ms{28800000U};

    // Optional dependencies
    std::unique_ptr<ILogger>  m_logger;
    std::unique_ptr<IDisplay> m_display;

    // FSM state
    AquariaState m_state;
    int          m_worsen_count;
    int          m_fault_count;

    // Actuator state (mirrored for display)
    bool     m_buzzer_on;
    bool     m_heater_on;
    uint64_t m_last_feed_ms;
};

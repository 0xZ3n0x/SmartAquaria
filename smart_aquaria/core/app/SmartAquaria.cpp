#include "SmartAquaria.h"

#include "ILogger.h"
#include "domain/IActuator.h"
#include "domain/IDisplay.h"
#include "domain/ISensor.h"
#include "domain/Readings.h"

namespace
{
// Maps operational states to a numeric severity for hysteresis comparison.
// SensorFault returns -1 and is intentionally excluded: its transitions are
// managed separately in readSensors(), not through the severity path.
int severityOf(AquariaState s) noexcept
{
    switch (s)
    {
    case AquariaState::Normal:  return 0;
    case AquariaState::Warning: return 1;
    case AquariaState::Alarm:   return 2;
    default:                    return -1;
    }
}
} // namespace

SmartAquaria::SmartAquaria(ISensor& sensor, IActuator& actuator, const Thresholds& thresholds,
                           IEventQueue& queue, IClock& clock, std::atomic<bool>& shutdown)
    : m_sensor(sensor)
    , m_actuator(actuator)
    , m_queue(queue)
    , m_clock(clock)
    , m_shutdown(shutdown)
    , m_thresholds(thresholds)
    , m_state(AquariaState::Normal)
    , m_worsen_count(0)
    , m_fault_count(0)
    , m_buzzer_on(false)
    , m_heater_on(false)
    , m_last_feed_ms(clock.now_ms())
{
}

SmartAquaria::~SmartAquaria()
{
    stop();
}

void SmartAquaria::setLogger(std::unique_ptr<ILogger> logger)
{
    m_logger = std::move(logger);
}

void SmartAquaria::setDisplay(std::unique_ptr<IDisplay> display)
{
    m_display = std::move(display);
}

void SmartAquaria::setFeedInterval(uint32_t interval_ms)
{
    m_feed_interval_ms = interval_ms;
}

void SmartAquaria::push(Event event)
{
    m_queue.push(static_cast<int>(event));
}

void SmartAquaria::run()
{
    log("run", "entering event loop");
    int raw{};
    while (!m_shutdown.load())
    {
        if (m_queue.wait(raw))
        {
            dispatch(static_cast<Event>(raw));
        }
    }
    log("run", "stopped");
}

void SmartAquaria::stop()
{
    m_shutdown.store(true);
    m_queue.shutdown();
}

AquariaState SmartAquaria::currentState() const noexcept
{
    return m_state;
}

void SmartAquaria::dispatch(Event event)
{
    switch (event)
    {
    case Event::SampleTimerElapsed:
        handleSample();
        break;
    }
}

void SmartAquaria::transit(AquariaState next)
{
    if (m_state == next)
    {
        return;
    }

    switch (m_state)
    {
    case AquariaState::Normal:
    case AquariaState::Warning:
        break;
    case AquariaState::Alarm:
    case AquariaState::SensorFault:
        log("buzzer", "OFF");
        m_actuator.buzz(false);
        m_buzzer_on = false;
        break;
    }

    m_state = next;

    switch (m_state)
    {
    case AquariaState::Normal:
    case AquariaState::Warning:
        break;
    case AquariaState::Alarm:
        log("buzzer", "ON");
        m_actuator.buzz(true);
        m_buzzer_on = true;
        break;
    case AquariaState::SensorFault:
        log("buzzer", "ON (sensor fault)");
        m_actuator.buzz(true);
        m_buzzer_on = true;
        m_actuator.heat(false);
        m_heater_on = false;
        break;
    }
}

void SmartAquaria::handleSample()
{
    const SensorData data = readSensors();

    if (!data.temp || !data.do_val)
    {
        callDisplay(data);
        return;
    }

    updateHeater(*data.temp);
    updateFSM(*data.temp, *data.do_val);
    checkFeed();
    callDisplay(data);
}

SmartAquaria::SensorData SmartAquaria::readSensors()
{
    log("sensor", "reading temperature");
    log("sensor", "reading DO");

    SensorData data{m_sensor.getTemperature(), m_sensor.getDO()};

    if (!data.temp || !data.do_val)
    {
        m_worsen_count = 0; // reset so a gap of failures doesn't accumulate toward hysteresis
        if (++m_fault_count >= kFaultThreshold && m_state != AquariaState::SensorFault)
        {
            transit(AquariaState::SensorFault);
        }
        return data;
    }

    if (m_state == AquariaState::SensorFault)
    {
        m_fault_count  = 0;
        m_worsen_count = 0;
        transit(AquariaState::Normal);
    }
    m_fault_count = 0;
    return data;
}

void SmartAquaria::updateHeater(float temp)
{
    const bool h = temp < m_thresholds.tempMin();
    m_actuator.heat(h);
    m_heater_on = h;
}

void SmartAquaria::updateFSM(float temp, float doVal)
{
    const AquariaState target = computeTarget(temp, doVal);

    if (severityOf(target) > severityOf(m_state))
    {
        // Worsening: require kHysteresisUp consecutive bad readings before transitioning.
        if (++m_worsen_count >= kHysteresisUp)
        {
            transit(target);
            m_worsen_count = 0;
        }
    }
    else
    {
        // Improving: transition immediately on the first good reading.
        m_worsen_count = 0;
        transit(target);
    }
}

void SmartAquaria::checkFeed()
{
    if (m_clock.now_ms() - m_last_feed_ms >= static_cast<uint64_t>(m_feed_interval_ms))
    {
        handleFeed();
    }
}

void SmartAquaria::handleFeed()
{
    // Only feed when the system is fully healthy; skip feeding during Warning/Alarm/SensorFault.
    if (m_state == AquariaState::Normal)
    {
        m_actuator.feed();
        m_last_feed_ms = m_clock.now_ms();
    }
}

AquariaState SmartAquaria::computeTarget(float temp, float doVal) const noexcept
{
    if (m_thresholds.isCritical(temp, doVal))
    {
        return AquariaState::Alarm;
    }
    if (m_thresholds.isWarning(temp, doVal))
    {
        return AquariaState::Warning;
    }
    return AquariaState::Normal;
}

void SmartAquaria::callDisplay(const SensorData& data) const
{
    if (m_display != nullptr)
    {
        m_display->display(m_state, buildReadings(data));
    }
}

Readings SmartAquaria::buildReadings(const SensorData& data) const
{
    Readings r;
    r.temperature       = data.temp;
    r.do_value          = data.do_val;
    r.buzzer_on         = m_buzzer_on;
    r.heater_on         = m_heater_on;
    r.time_until_feed_s = feedTimeRemainingSeconds();
    return r;
}

uint32_t SmartAquaria::feedTimeRemainingSeconds() const
{
    const uint64_t elapsed_ms = m_clock.now_ms() - m_last_feed_ms;

    if (elapsed_ms >= static_cast<uint64_t>(m_feed_interval_ms))
    {
        return 0U;
    }

    return static_cast<uint32_t>((static_cast<uint64_t>(m_feed_interval_ms) - elapsed_ms) / 1000U);
}

void SmartAquaria::log(const char* prefix, const char* message) const
{
    if (m_logger != nullptr)
    {
        m_logger->log(prefix, message);
    }
}

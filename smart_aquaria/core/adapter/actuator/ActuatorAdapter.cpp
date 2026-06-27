#include "ActuatorAdapter.h"

#include "device/actuator/buzzer/IBuzzer.h"
#include "device/actuator/heater/IHeater.h"

ActuatorAdapter::ActuatorAdapter(IBuzzer& buzzer, IHeater& heater) : m_buzzer(buzzer), m_heater(heater)
{
}

bool ActuatorAdapter::buzz(bool on)
{
    return m_buzzer.buzz(on);
}

bool ActuatorAdapter::heat(bool on)
{
    return m_heater.heat(on);
}

void ActuatorAdapter::feed()
{
    // No feeding hardware connected; intentional no-op.
}

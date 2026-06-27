#include "IGPIO.h"

#include "HeaterDevice.h"

HeaterDevice::HeaterDevice(IGPIO& gpio) : m_gpio(gpio) {}

bool HeaterDevice::heat(bool on)
{
    return m_gpio.write(on);
}

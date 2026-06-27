#include "IGPIO.h"

#include "BuzzerDevice.h"

BuzzerDevice::BuzzerDevice(IGPIO& gpio) : m_gpio(gpio) {}

bool BuzzerDevice::buzz(bool on)
{
    return m_gpio.write(on);
}

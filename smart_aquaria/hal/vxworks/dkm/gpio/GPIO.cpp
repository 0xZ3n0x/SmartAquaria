#include "GPIO.h"

#include <stdexcept>
#include <string>

#include <vxWorks.h>
#include <subsys/gpio/vxbGpioLib.h>

GPIO::GPIO(uint32_t pin) : m_pin(pin), m_open(false)
{
    if (vxbGpioAlloc(pin) != OK)
        throw std::runtime_error("GPIO: vxbGpioAlloc failed for pin " + std::to_string(pin));

    if (vxbGpioSetDir(pin, GPIO_DIR_OUTPUT) != OK)
    {
        vxbGpioFree(pin);
        throw std::runtime_error("GPIO: vxbGpioSetDir failed for pin " + std::to_string(pin));
    }

    m_open = true;
}

GPIO::~GPIO()
{
    close();
}

bool GPIO::write(bool active)
{
    return vxbGpioSetValue(m_pin, active ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW) == OK;
}

bool GPIO::isOpen() const
{
    return m_open;
}

void GPIO::close()
{
    if (m_open)
    {
        vxbGpioFree(m_pin);
        m_open = false;
    }
}

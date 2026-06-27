#include <cstdlib>
#include <optional>

#include "IUART.h"
#include "EzoRTDSensorDevice.h"

EzoRTDSensorDevice::EzoRTDSensorDevice(IUART& uart) : m_uart(uart), m_buffer{} {}

std::optional<float> EzoRTDSensorDevice::getTemperature()
{
    if (!m_uart.isOpen())
    {
        return std::nullopt;
    }

    m_uart.flush();

    if (m_uart.write("R\n", 2) != 2)
    {
        return std::nullopt;
    }

    // Packet format: "XX.XX\r\n" — accumulate until \n or timeout
    size_t len = 0;
    while (len < m_buffer.size() - 1)
    {
        if (m_uart.read(m_buffer.data() + len, 1) <= 0)
        {
            break;
        }
        const char ch = m_buffer[len++];
        if (ch == '\n')
        {
            break;
        }
    }

    // Strip trailing \r\n
    while (len > 0 && (m_buffer[len - 1] == '\r' || m_buffer[len - 1] == '\n'))
    {
        --len;
    }
    m_buffer[len] = '\0';

    if (len == 0)
    {
        return std::nullopt;
    }

    char* end{};
    const float val = strtof(m_buffer.data(), &end);
    if (end != m_buffer.data() && end != nullptr && *end == '\0')
    {
        return val;
    }

    return std::nullopt;
}

#include "GPIO.h"

#include <array>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <unistd.h>

GPIO::GPIO(const std::string& chip_path, unsigned int line_offset)
    : m_chip_fd(::open(chip_path.c_str(), O_RDONLY | O_CLOEXEC)), m_req_fd(-1), m_line_offset(line_offset)
{
    if (m_chip_fd < 0)
    {
        throw std::runtime_error(std::string("Failed to open GPIO chip: ") + chip_path);
    }

    struct gpio_v2_line_request req{};
    req.offsets[0] = line_offset;
    req.num_lines  = 1;

    // Consumer label is fixed; the kernel uses it only for debugging via gpioinfo.
    const std::array<char, 20> consumer = {'s', 'm', 'a', 'r', 't', '-', 'a', 'q', 'u', 'a',
                                           'r', 'i', 'a', '-', 'b', 'u', 'z', 'z', 'e', 'r'};
    for (std::size_t i = 0; i < consumer.size() && i < GPIO_MAX_NAME_SIZE; ++i)
    {
        req.consumer[i] = consumer[i];
    }

    req.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;

    if (::ioctl(m_chip_fd, GPIO_V2_GET_LINE_IOCTL, &req) < 0)
    {
        ::close(m_chip_fd);
        m_chip_fd = -1;
        throw std::runtime_error("GPIO_V2_GET_LINE_IOCTL failed");
    }

    m_req_fd = req.fd;
}

GPIO::~GPIO()
{
    close();
}

void GPIO::close()
{
    if (m_req_fd >= 0)
    {
        ::close(m_req_fd);
        m_req_fd = -1;
    }
    if (m_chip_fd >= 0)
    {
        ::close(m_chip_fd);
        m_chip_fd = -1;
    }
}

bool GPIO::write(bool active)
{
    if (m_req_fd < 0)
    {
        return false;
    }

    struct gpio_v2_line_values vals{};
    vals.mask = 1U;
    vals.bits = active ? 1U : 0U;

    return ::ioctl(m_req_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &vals) == 0;
}

bool GPIO::isOpen() const
{
    return m_req_fd >= 0;
}

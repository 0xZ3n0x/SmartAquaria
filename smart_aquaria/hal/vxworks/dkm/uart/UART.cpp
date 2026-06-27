#include "UART.h"

#include <stdexcept>

#include <fcntl.h>
#include <ioLib.h>      // FIOBAUDRATE, FIOSETOPTIONS, FIORFLUSH, OPT_RAW
#include <selectLib.h>  // select(), fd_set, timeval
#include <unistd.h>

UART::UART(const std::string& port, int baudrate, float timeout)
    : m_fd(open(port.c_str(), O_RDWR, 0))
    , m_timeout_ms(static_cast<int>(timeout * 1000.0f))
{
    if (m_fd < 0)
    {
        throw std::runtime_error(std::string("Failed to open port: ") + port);
    }
    if (!configure(baudrate))
    {
        ::close(m_fd);
        m_fd = -1;
        throw std::runtime_error("Failed to configure UART");
    }
}

UART::~UART()
{
    close();
}

void UART::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

int UART::write(const char* data, size_t len)
{
    if (m_fd < 0)
        return -1;
    return static_cast<int>(::write(m_fd, data, len));
}

int UART::read(char* buffer, size_t len)
{
    if (m_fd < 0)
        return -1;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);

    struct timeval tv;
    tv.tv_sec  = m_timeout_ms / 1000;
    tv.tv_usec = static_cast<long>((m_timeout_ms % 1000) * 1000);

    const int ready = select(m_fd + 1, &readfds, nullptr, nullptr, &tv);
    if (ready <= 0)
        return 0;

    return static_cast<int>(::read(m_fd, buffer, len));
}

void UART::flush()
{
    if (m_fd >= 0)
        ioctl(m_fd, FIORFLUSH, 0);
}

bool UART::isOpen() const
{
    return m_fd >= 0;
}

bool UART::configure(int baudrate) const
{
    if (ioctl(m_fd, FIOBAUDRATE, baudrate) == ERROR)
        return false;

    if (ioctl(m_fd, FIOSETOPTIONS, OPT_RAW) == ERROR)
        return false;

    return true;
}

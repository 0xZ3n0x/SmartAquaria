#include "UART.h"

#include <stdexcept>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

UART::UART(const std::string& port, int baudrate, float timeout) : m_fd(open(port.c_str(), O_RDWR | O_NOCTTY))
{
    if (m_fd < 0)
    {
        throw std::runtime_error(std::string("Failed to open port: ") + port);
    }

    const auto configResult = configure(baudrate, timeout);
    if (!configResult)
    {
        close();
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
    {
        return -1;
    }
    return static_cast<int>(::write(m_fd, data, len));
}

int UART::read(char* buffer, size_t len)
{
    if (m_fd < 0)
    {
        return -1;
    }
    return static_cast<int>(::read(m_fd, buffer, len));
}

void UART::flush()
{
    if (m_fd >= 0)
    {
        tcflush(m_fd, TCIOFLUSH);
    }
}

bool UART::isOpen() const
{
    return m_fd >= 0;
}

bool UART::configure(int baudrate, float timeout) const
{
    auto getBaudrate = [](int baud) -> int
    {
        switch (baud)
        {
        case 9600:   return B9600;
        case 115200: return B115200;
        case 57600:  return B57600;
        case 38400:  return B38400;
        case 19200:  return B19200;
        default:     return -1;
        }
    };

    const int baudrateVal = getBaudrate(baudrate);
    if (baudrateVal < 0)
    {
        return false;
    }

    struct termios tty{};

    if (tcgetattr(m_fd, &tty) != 0)
    {
        return false;
    }

    cfsetospeed(&tty, static_cast<speed_t>(baudrateVal));
    cfsetispeed(&tty, static_cast<speed_t>(baudrateVal));

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag     = 0;
    tty.c_oflag     = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = static_cast<cc_t>(timeout * 10); // VTIME unit is deciseconds (0.1 s), not milliseconds

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    return tcsetattr(m_fd, TCSANOW, &tty) == 0;
}

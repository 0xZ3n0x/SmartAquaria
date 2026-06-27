#include "SPI.h"

#include <stdexcept>

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

SPI::SPI(const std::string& device, uint32_t speed_hz, uint8_t mode, uint8_t bits_per_word)
    : m_fd(open(device.c_str(), O_RDWR))
{
    if (m_fd < 0)
    {
        throw std::runtime_error(std::string("Failed to open SPI device: ") + device);
    }

    if (!configure(speed_hz, mode, bits_per_word))
    {
        close();
        throw std::runtime_error("Failed to configure SPI");
    }
}

SPI::~SPI()
{
    close();
}

void SPI::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool SPI::transfer(const uint8_t* tx, uint8_t* rx, size_t len)
{
    if (m_fd < 0)
    {
        return false;
    }

    struct spi_ioc_transfer tr{};
    tr.tx_buf        = reinterpret_cast<unsigned long>(tx);
    tr.rx_buf        = reinterpret_cast<unsigned long>(rx);
    tr.len           = static_cast<uint32_t>(len);
    tr.speed_hz      = 0; // 0 = use the speed set during configure()
    tr.bits_per_word = 0; // 0 = use bits_per_word set during configure()

    return ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr) >= 0;
}

bool SPI::isOpen() const
{
    return m_fd >= 0;
}

bool SPI::configure(uint32_t speed_hz, uint8_t mode, uint8_t bits_per_word) const
{
    if (ioctl(m_fd, SPI_IOC_WR_MODE, &mode) < 0)
    {
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0)
    {
        return false;
    }

    if (ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) < 0)
    {
        return false;
    }

    return true;
}

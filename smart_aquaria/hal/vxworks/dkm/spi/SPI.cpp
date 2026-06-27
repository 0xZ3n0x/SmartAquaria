#include "SPI.h"

#include <stdexcept>
#include <string>

static constexpr const char* kSpiSlaveName = "st7789";

SPI::SPI(uint32_t ctrlIndex, uint32_t speed, uint8_t chipSelect, int mode)
    : m_slavedev(nullptr), m_hw{}
{
    m_slavedev = vxbDevAcquireByName(const_cast<char*>(kSpiSlaveName),
                                     static_cast<UINT32>(ctrlIndex));
    if (m_slavedev == nullptr)
        throw std::runtime_error(
            std::string("SPI: vxbDevAcquireByName(\"") + kSpiSlaveName +
            "\", " + std::to_string(ctrlIndex) + ") failed");

    m_hw.chipSelect = chipSelect;
    m_hw.bitWidth   = 8;
    m_hw.devFreq    = speed;
    m_hw.mode       = static_cast<UINT32>(mode);
    m_hw.dataLines  = 1;
}

SPI::~SPI()
{
    close();
}

bool SPI::transfer(const uint8_t* tx, uint8_t* rx, size_t len)
{
    SPI_TRANSFER xfer{};
    xfer.txBuf = reinterpret_cast<UINT8*>(const_cast<uint8_t*>(tx));
    xfer.rxBuf = rx;
    xfer.txLen = (tx != nullptr) ? static_cast<UINT32>(len) : 0U;
    xfer.rxLen = (rx != nullptr) ? static_cast<UINT32>(len) : 0U;
    return vxbSpiDevXfer(m_slavedev, &m_hw, &xfer) == OK;
}

bool SPI::isOpen() const
{
    return m_slavedev != nullptr;
}

void SPI::close()
{
    if (m_slavedev != nullptr)
    {
        vxbDevRelease(m_slavedev);
        m_slavedev = nullptr;
    }
}

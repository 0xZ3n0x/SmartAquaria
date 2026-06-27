#pragma once

#include <cstdint>

#include <vxWorks.h>
#include <hwif/vxbus/vxbLib.h>
#include <hwif/buslib/vxbSpiLib.h>

#include "ISPI.h"

// Uses vxbDevAcquireByName to locate the SPI slave device enumerated from the
// FDT child node (st7789@0 under spi@fe204000), then performs transfers with
// vxbSpiDevXfer.  Does NOT require _WRS_CONFIG_VXBUS_SPI_IOS.
// ctrlIndex: instance index of the slave device (0 = st7789@0 / CS0)
class SPI final : public ISPI
{
  public:
    SPI(uint32_t ctrlIndex, uint32_t speed, uint8_t chipSelect, int mode = 0);
    ~SPI() override;

    SPI(const SPI&)            = delete;
    SPI& operator=(const SPI&) = delete;
    SPI(SPI&&)                 = delete;
    SPI& operator=(SPI&&)      = delete;

    void close() override;
    bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) override;
    bool isOpen() const override;

  private:
    VXB_DEV_ID   m_slavedev;
    SPI_HARDWARE m_hw;
};

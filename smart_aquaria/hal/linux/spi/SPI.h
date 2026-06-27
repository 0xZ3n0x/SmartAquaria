#pragma once

#include <cstdint>
#include <string>

#include "ISPI.h"

class SPI final : public ISPI
{
  public:
    SPI(const std::string& device, uint32_t speed_hz, uint8_t mode = 0, uint8_t bits_per_word = 8);
    ~SPI() override;

    SPI(const SPI&)            = delete;
    SPI& operator=(const SPI&) = delete;
    SPI(SPI&&)                 = delete;
    SPI& operator=(SPI&&)      = delete;

    void close() override;
    bool transfer(const uint8_t* tx, uint8_t* rx, size_t len) override;
    bool isOpen() const override;

  private:
    bool configure(uint32_t speed_hz, uint8_t mode, uint8_t bits_per_word) const;

    int m_fd;
};

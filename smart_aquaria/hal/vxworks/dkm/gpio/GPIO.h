#pragma once

#include <cstdint>

#include "IGPIO.h"

// Uses vxbGpioLib direct kernel API (DKM only).
// Requires VIP component INCLUDE_GPIO_DRV.
class GPIO final : public IGPIO
{
  public:
    explicit GPIO(uint32_t pin);
    ~GPIO() override;

    GPIO(const GPIO&)            = delete;
    GPIO& operator=(const GPIO&) = delete;
    GPIO(GPIO&&)                 = delete;
    GPIO& operator=(GPIO&&)      = delete;

    bool write(bool active) override;
    bool isOpen() const override;
    void close() override;

  private:
    uint32_t m_pin;
    bool     m_open;
};

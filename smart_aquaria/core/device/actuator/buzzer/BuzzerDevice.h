#pragma once

#include "IBuzzer.h"

class IGPIO;

class BuzzerDevice final : public IBuzzer
{
  public:
    explicit BuzzerDevice(IGPIO& gpio);
    ~BuzzerDevice() = default;

    BuzzerDevice(const BuzzerDevice&) = delete;
    BuzzerDevice& operator=(const BuzzerDevice&) = delete;
    BuzzerDevice(BuzzerDevice&&) = delete;
    BuzzerDevice& operator=(BuzzerDevice&&) = delete;

    bool buzz(bool on) override;

  private:
    IGPIO& m_gpio;
};

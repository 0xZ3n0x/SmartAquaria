#pragma once

#include "IHeater.h"

class IGPIO;

class HeaterDevice final : public IHeater
{
  public:
    explicit HeaterDevice(IGPIO& gpio);
    ~HeaterDevice() = default;

    HeaterDevice(const HeaterDevice&) = delete;
    HeaterDevice& operator=(const HeaterDevice&) = delete;
    HeaterDevice(HeaterDevice&&) = delete;
    HeaterDevice& operator=(HeaterDevice&&) = delete;

    bool heat(bool on) override;

  private:
    IGPIO& m_gpio;
};

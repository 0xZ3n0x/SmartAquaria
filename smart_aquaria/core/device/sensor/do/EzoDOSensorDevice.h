#pragma once

#include <array>

#include "IDOSensor.h"

class IUART;

class EzoDOSensorDevice final : public IDOSensor
{
  public:
    explicit EzoDOSensorDevice(IUART& uart);
    ~EzoDOSensorDevice() = default;

    EzoDOSensorDevice(const EzoDOSensorDevice&) = delete;
    EzoDOSensorDevice& operator=(const EzoDOSensorDevice&) = delete;
    EzoDOSensorDevice(EzoDOSensorDevice&&) = delete;
    EzoDOSensorDevice& operator=(EzoDOSensorDevice&&) = delete;

    std::optional<float> getDO() override;

  private:
    IUART& m_uart;
    std::array<char, 256> m_buffer;
};

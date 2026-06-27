#pragma once

#include <array>

#include "ITemperatureSensor.h"

class IUART;

class EzoRTDSensorDevice final : public ITemperatureSensor
{
  public:
    explicit EzoRTDSensorDevice(IUART& uart);
    ~EzoRTDSensorDevice() = default;

    EzoRTDSensorDevice(const EzoRTDSensorDevice&) = delete;
    EzoRTDSensorDevice& operator=(const EzoRTDSensorDevice&) = delete;
    EzoRTDSensorDevice(EzoRTDSensorDevice&&) = delete;
    EzoRTDSensorDevice& operator=(EzoRTDSensorDevice&&) = delete;

    std::optional<float> getTemperature() override;

  private:
    IUART& m_uart;
    std::array<char, 256> m_buffer;
};

#pragma once

#include "domain/ISensor.h"

class ITemperatureSensor;
class IDOSensor;

class SensorAdapter final : public ISensor
{
  public:
    explicit SensorAdapter(ITemperatureSensor& tempDevice, IDOSensor& doDevice);
    ~SensorAdapter() = default;

    SensorAdapter(const SensorAdapter&) = delete;
    SensorAdapter& operator=(const SensorAdapter&) = delete;
    SensorAdapter(SensorAdapter&&) = delete;
    SensorAdapter& operator=(SensorAdapter&&) = delete;

    std::optional<float> getTemperature() override;
    std::optional<float> getDO() override;

  private:
    ITemperatureSensor& m_temp_device;
    IDOSensor& m_do_device;
};

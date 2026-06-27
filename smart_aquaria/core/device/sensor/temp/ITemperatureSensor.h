#pragma once

#include <optional>

class ITemperatureSensor
{
  public:
    [[nodiscard]] virtual std::optional<float> getTemperature() = 0;
    virtual ~ITemperatureSensor() = default;
};

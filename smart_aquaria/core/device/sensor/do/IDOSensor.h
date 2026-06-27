#pragma once

#include <optional>

class IDOSensor
{
  public:
    [[nodiscard]] virtual std::optional<float> getDO() = 0;
    virtual ~IDOSensor() = default;
};

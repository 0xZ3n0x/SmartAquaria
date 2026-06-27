#pragma once

#include <optional>

class ISensor
{
public:
    virtual ~ISensor() = default;

    [[nodiscard]] virtual std::optional<float> getTemperature() = 0;
    [[nodiscard]] virtual std::optional<float> getDO() = 0;
};

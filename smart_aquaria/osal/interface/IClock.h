#pragma once

#include <cstdint>

class IClock
{
  public:
    virtual ~IClock() = default;

    [[nodiscard]] virtual uint64_t now_ms() const noexcept = 0;
    virtual void sleep_ms(uint32_t ms) const = 0;
};

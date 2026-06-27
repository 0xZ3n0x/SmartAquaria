#pragma once

#include <chrono>
#include <cstdint>

#include "IClock.h"

class Clock : public IClock
{
  public:
    Clock();
    ~Clock() override = default;

    Clock(const Clock&)            = delete;
    Clock& operator=(const Clock&) = delete;
    Clock(Clock&&)                 = delete;
    Clock& operator=(Clock&&)      = delete;

    [[nodiscard]] uint64_t now_ms() const noexcept override;
    void sleep_ms(uint32_t ms) const override;

  private:
    std::chrono::steady_clock::time_point m_epoch;
};

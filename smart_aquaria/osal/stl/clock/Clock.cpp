#include <thread>

#include "Clock.h"

Clock::Clock() : m_epoch(std::chrono::steady_clock::now()) {}

uint64_t Clock::now_ms() const noexcept
{
    const auto elapsed = std::chrono::steady_clock::now() - m_epoch;
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

void Clock::sleep_ms(uint32_t ms) const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

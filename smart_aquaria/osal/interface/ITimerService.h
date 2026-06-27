#pragma once

#include <cstdint>
#include <functional>

class ITimerService
{
  public:
    virtual ~ITimerService() = default;

    virtual void setInterval(uint32_t interval_ms) = 0;

    virtual void run(std::function<void()> callback) = 0;

    virtual void stop() = 0;
};

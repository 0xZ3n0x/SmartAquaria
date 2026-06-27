#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

#include "ITimerService.h"

class TimerService : public ITimerService
{
  public:
    TimerService();
    ~TimerService() override;

    TimerService(const TimerService&)            = delete;
    TimerService& operator=(const TimerService&) = delete;
    TimerService(TimerService&&)                 = delete;
    TimerService& operator=(TimerService&&)      = delete;

    void setInterval(uint32_t interval_ms) override;
    void run(std::function<void()> callback) override;
    void stop() override;

  private:
    void stopImpl();

    std::atomic<uint32_t>   m_interval_ms{50};
    std::condition_variable m_cv;
    std::mutex              m_mutex;
    std::atomic<bool>       m_running{false};
    std::thread             m_thread;
};

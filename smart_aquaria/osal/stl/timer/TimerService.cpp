#include "TimerService.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

TimerService::TimerService() = default;

TimerService::~TimerService()
{
    stopImpl();
}

void TimerService::setInterval(uint32_t interval_ms)
{
    m_interval_ms = interval_ms;
}

void TimerService::run(std::function<void()> callback)
{
    if (m_thread.joinable())
    {
        stop();
    }

    m_running = true;

    m_thread = std::thread(
        [this, callback = std::move(callback)]()
        {
            while (true)
            {
                std::unique_lock lock(m_mutex);
                // wait_for returns true when the predicate fires (stop requested),
                // false when it times out (interval elapsed → fire callback).
                const bool stopped = m_cv.wait_for(lock, std::chrono::milliseconds(m_interval_ms),
                                                         [this]() { return !m_running.load(); });

                if (stopped)
                {
                    break;
                }
                callback();
            }
        });
}

void TimerService::stop()
{
    stopImpl();
}

void TimerService::stopImpl()
{
    m_running = false;
    m_cv.notify_all();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

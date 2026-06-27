#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "IEventQueue.h"

class EventQueue : public IEventQueue
{
  public:
    EventQueue();
    ~EventQueue() override;

    EventQueue(const EventQueue&)            = delete;
    EventQueue& operator=(const EventQueue&) = delete;
    EventQueue(EventQueue&&)                 = delete;
    EventQueue& operator=(EventQueue&&)      = delete;

    void push(int event) override;
    bool wait(int& out) override;
    void shutdown() override;

  private:
    void shutdownImpl();

    std::queue<int>         m_events;
    std::mutex              m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool>       m_shutdown_flag{false};
};

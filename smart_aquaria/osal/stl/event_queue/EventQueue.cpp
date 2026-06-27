#include "EventQueue.h"

EventQueue::EventQueue() = default;

EventQueue::~EventQueue()
{
    shutdownImpl();
}

void EventQueue::push(int event)
{
    {
        std::lock_guard lock(m_mutex);
        m_events.push(event);
    }
    m_cv.notify_one();
}

bool EventQueue::wait(int& out)
{
    std::unique_lock lock(m_mutex);
    m_cv.wait(lock, [this]() { return !m_events.empty() || m_shutdown_flag.load(); });

    if (m_events.empty())
    {
        return false;
    }

    out = m_events.front();
    m_events.pop();
    return true;
}

void EventQueue::shutdown()
{
    shutdownImpl();
}

void EventQueue::shutdownImpl()
{
    m_shutdown_flag = true;
    m_cv.notify_all();
}

#pragma once

class IEventQueue
{
  public:
    virtual ~IEventQueue() = default;

    virtual void push(int event) = 0;
    virtual bool wait(int& out)  = 0;
    virtual void shutdown()      = 0;
};

#pragma once

class IBuzzer
{
  public:
    virtual bool buzz(bool on) = 0;
    virtual ~IBuzzer() = default;
};

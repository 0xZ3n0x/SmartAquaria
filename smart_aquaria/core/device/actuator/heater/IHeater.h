#pragma once

class IHeater
{
  public:
    virtual bool heat(bool on) = 0;
    virtual ~IHeater() = default;
};

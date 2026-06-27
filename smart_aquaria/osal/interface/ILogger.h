#pragma once

class ILogger
{
  public:
    virtual ~ILogger() = default;

    virtual bool log(const char* prefix, const char* message) = 0;
};

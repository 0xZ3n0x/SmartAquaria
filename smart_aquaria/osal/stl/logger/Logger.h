#pragma once

#include "ILogger.h"

class Logger final : public ILogger
{
  public:
    explicit Logger(const char* filename);
    ~Logger() override;

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&)                 = delete;
    Logger& operator=(Logger&&)      = delete;

    bool log(const char* prefix, const char* message) override;

  private:
    int m_fd;
};

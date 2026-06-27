#pragma once

#include <string>

#include "IUART.h"

class UART final : public IUART
{
  public:
    UART(const std::string& port, int baudrate, float timeout);
    ~UART() override;

    UART(const UART&)            = delete;
    UART& operator=(const UART&) = delete;
    UART(UART&&)                 = delete;
    UART& operator=(UART&&)      = delete;

    void close() override;
    int  write(const char* data, size_t len) override;
    int  read(char* buffer, size_t len) override;
    void flush() override;
    bool isOpen() const override;

  private:
    bool configure(int baudrate, float timeout) const;

    int m_fd;
};

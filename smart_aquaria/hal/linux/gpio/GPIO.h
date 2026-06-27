#pragma once

#include <string>

#include "IGPIO.h"

class GPIO final : public IGPIO
{
  public:
    GPIO(const std::string& chip_path, unsigned int line_offset);
    ~GPIO() override;

    GPIO(const GPIO&)            = delete;
    GPIO& operator=(const GPIO&) = delete;
    GPIO(GPIO&&)                 = delete;
    GPIO& operator=(GPIO&&)      = delete;

    bool write(bool active) override;
    bool isOpen() const override;
    void close() override;

  private:
    int          m_chip_fd;
    int          m_req_fd;
    unsigned int m_line_offset;
};

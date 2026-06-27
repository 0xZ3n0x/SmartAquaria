#pragma once

#include <fstream>

#include "device/display/IDisplayDevice.h"

class TerminalDisplayDevice final : public IDisplayDevice
{
  public:
    explicit TerminalDisplayDevice(const char* filename);
    ~TerminalDisplayDevice() override = default;

    TerminalDisplayDevice(const TerminalDisplayDevice&)            = delete;
    TerminalDisplayDevice& operator=(const TerminalDisplayDevice&) = delete;
    TerminalDisplayDevice(TerminalDisplayDevice&&)                 = delete;
    TerminalDisplayDevice& operator=(TerminalDisplayDevice&&)      = delete;

    void clear() override;
    void print(const char* text) override;
    void printStyled(const char* text, DisplayStyle style) override;
    void flush() override;

  private:
    std::ofstream m_file;
};

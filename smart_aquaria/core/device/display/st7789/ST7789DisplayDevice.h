#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "device/display/IDisplayDevice.h"

class IClock;
class IGPIO;
class ISPI;

class ST7789DisplayDevice final : public IDisplayDevice
{
  public:
    ST7789DisplayDevice(ISPI& spi, IGPIO& dc, IGPIO& rst, IClock& clock);
    ~ST7789DisplayDevice();

    ST7789DisplayDevice(const ST7789DisplayDevice&)            = delete;
    ST7789DisplayDevice& operator=(const ST7789DisplayDevice&) = delete;
    ST7789DisplayDevice(ST7789DisplayDevice&&)                 = delete;
    ST7789DisplayDevice& operator=(ST7789DisplayDevice&&)      = delete;

    void clear() override;
    void print(const char* text) override;
    void printStyled(const char* text, DisplayStyle style) override;
    void flush() override;

  private:
    ISPI&   m_spi;
    IGPIO&  m_dc;
    IGPIO&  m_rst;
    IClock& m_clock;

    uint16_t m_cursor_x;
    uint16_t m_cursor_y;

    std::vector<uint8_t> m_rowbuf;

    void     initDisplay();
    void     sendCommand(uint8_t cmd);
    void     sendData(uint8_t data);
    void     sendData(const uint8_t* data, size_t len);
    void     setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void     drawChar(uint16_t x, char c, uint16_t fg_color);
    void     flushRow();
    [[nodiscard]] static uint16_t styleToColor(DisplayStyle style) noexcept;
};

#include "ST7789DisplayDevice.h"

#include <algorithm>
#include <array>

#include "Font8x8.hpp"
#include "IClock.h"
#include "IGPIO.h"
#include "ISPI.h"

// ST7789 command codes
namespace
{
constexpr uint8_t k_cmd_swreset   = 0x01;
constexpr uint8_t k_cmd_slpout    = 0x11;
constexpr uint8_t k_cmd_noron     = 0x13;
constexpr uint8_t k_cmd_invon     = 0x21;
constexpr uint8_t k_cmd_dispon    = 0x29;
constexpr uint8_t k_cmd_caset     = 0x2A;
constexpr uint8_t k_cmd_raset     = 0x2B;
constexpr uint8_t k_cmd_ramwr     = 0x2C;
constexpr uint8_t k_cmd_madctl    = 0x36;
constexpr uint8_t k_cmd_colmod    = 0x3A;
constexpr uint8_t k_cmd_porctrl   = 0xB2;
constexpr uint8_t k_cmd_gctrl     = 0xB7;
constexpr uint8_t k_cmd_vcoms     = 0xBB;
constexpr uint8_t k_cmd_lcmctrl   = 0xC0;
constexpr uint8_t k_cmd_vdvvrhen  = 0xC2;
constexpr uint8_t k_cmd_vrh       = 0xC3;
constexpr uint8_t k_cmd_vdv       = 0xC4;
constexpr uint8_t k_cmd_frctrl2   = 0xC6;
constexpr uint8_t k_cmd_pwctrl1   = 0xD0;
constexpr uint8_t k_cmd_pvgamctrl = 0xE0;
constexpr uint8_t k_cmd_nvgamctrl = 0xE1;

// Display geometry
constexpr uint16_t k_width       = 240;
constexpr uint16_t k_height      = 240;
constexpr uint8_t  k_font_w      = 8;
constexpr uint8_t  k_font_h      = 8;
constexpr uint8_t  k_scale_x     = 2;
constexpr uint8_t  k_scale_y     = 2;
constexpr uint8_t  k_char_w      = k_font_w * k_scale_x; // 16
constexpr uint8_t  k_char_h      = k_font_h * k_scale_y; // 16
constexpr size_t   k_rowbuf_size = static_cast<size_t>(k_width) * k_char_h * 2; // 7 680

// Display constants
constexpr uint8_t k_pixel_format    = 0x05;
constexpr uint8_t k_gate_control    = 0x14;
constexpr uint8_t k_vcom            = 0x37;
constexpr uint8_t k_lcm_control     = 0x2C;
constexpr uint8_t k_vdv_vrh_enable  = 0x01;
constexpr uint8_t k_vrh             = 0x12;
constexpr uint8_t k_vdv             = 0x20;
constexpr uint8_t k_frame_rate      = 0x0F;
constexpr size_t  k_spi_chunk       = 4096;
constexpr uint8_t k_font_char_limit = 128;
constexpr uint8_t k_display_on      = 0x7E;

// Color definitions (RGB565)
constexpr uint16_t k_color_red    = 0xF800;
constexpr uint16_t k_color_yellow = 0xFFE0;
constexpr uint16_t k_color_green  = 0x07E0;
constexpr uint16_t k_color_cyan   = 0x07FF;
constexpr uint16_t k_color_blue   = 0x4208;
constexpr uint16_t k_color_white  = 0xFFFF;
} // namespace

ST7789DisplayDevice::ST7789DisplayDevice(ISPI& spi, IGPIO& dc, IGPIO& rst, IClock& clock)
    : m_spi(spi), m_dc(dc), m_rst(rst), m_clock(clock), m_cursor_x(0), m_cursor_y(0),
      m_rowbuf(k_rowbuf_size, 0)
{
    initDisplay();
}

ST7789DisplayDevice::~ST7789DisplayDevice() = default;

void ST7789DisplayDevice::initDisplay()
{
    // Hardware reset pulse
    m_rst.write(true);
    m_clock.sleep_ms(500);
    m_rst.write(false);
    m_clock.sleep_ms(500);
    m_rst.write(true);
    m_clock.sleep_ms(500);

    // Software reset — wait 150 ms before sending further commands
    sendCommand(k_cmd_swreset);
    m_clock.sleep_ms(150);

    // Memory access control
    sendCommand(k_cmd_madctl);
    sendData(uint8_t{0x00});

    // Porch setting
    {
        sendCommand(k_cmd_porctrl);
        const std::array<uint8_t, 5> d = {0x0C, 0x0C, 0x00, 0x33, 0x33};
        sendData(d.data(), d.size());
    }

    // Pixel format: 16-bit RGB565
    sendCommand(k_cmd_colmod);
    sendData(k_pixel_format);

    // Gate control
    sendCommand(k_cmd_gctrl);
    sendData(k_gate_control);

    // VCOM setting
    sendCommand(k_cmd_vcoms);
    sendData(k_vcom);

    // LCM control
    sendCommand(k_cmd_lcmctrl);
    sendData(k_lcm_control);

    // VDV and VRH command enable
    sendCommand(k_cmd_vdvvrhen);
    sendData(k_vdv_vrh_enable);

    // VRH set
    sendCommand(k_cmd_vrh);
    sendData(k_vrh);

    // VDV set
    sendCommand(k_cmd_vdv);
    sendData(k_vdv);

    // Power control
    {
        sendCommand(k_cmd_pwctrl1);
        const std::array<uint8_t, 2> d = {0xA4, 0xA1};
        sendData(d.data(), d.size());
    }

    // Frame rate control
    sendCommand(k_cmd_frctrl2);
    sendData(k_frame_rate);

    // Positive voltage gamma
    {
        sendCommand(k_cmd_pvgamctrl);
        const std::array<uint8_t, 14> d = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
        sendData(d.data(), d.size());
    }

    // Negative voltage gamma
    {
        sendCommand(k_cmd_nvgamctrl);
        const std::array<uint8_t, 14> d = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
        sendData(d.data(), d.size());
    }

    // Display inversion on (required for IPS panels)
    sendCommand(k_cmd_invon);

    // Exit sleep mode — must come after register configuration
    sendCommand(k_cmd_slpout);
    m_clock.sleep_ms(100);

    // Normal display mode on
    sendCommand(k_cmd_noron);

    // Display on
    sendCommand(k_cmd_dispon);
    m_clock.sleep_ms(100);
}

void ST7789DisplayDevice::sendCommand(uint8_t cmd)
{
    m_dc.write(false);
    m_spi.transfer(&cmd, nullptr, 1);
}

void ST7789DisplayDevice::sendData(uint8_t data)
{
    m_dc.write(true);
    m_spi.transfer(&data, nullptr, 1);
}

void ST7789DisplayDevice::sendData(const uint8_t* data, size_t len)
{
    m_dc.write(true);
    m_spi.transfer(data, nullptr, len);
}

void ST7789DisplayDevice::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    sendCommand(k_cmd_caset);
    const std::array<uint8_t, 4> col_data = {static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0 & 0xFF),
                                              static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1 & 0xFF)};
    sendData(col_data.data(), col_data.size());

    sendCommand(k_cmd_raset);
    const std::array<uint8_t, 4> row_data = {static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0 & 0xFF),
                                              static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1 & 0xFF)};
    sendData(row_data.data(), row_data.size());

    sendCommand(k_cmd_ramwr);
}


void ST7789DisplayDevice::drawChar(uint16_t x, char c, uint16_t fg_color)
{
    const auto cidx = static_cast<uint8_t>(c);
    if (cidx >= k_font_char_limit)
    {
        return;
    }

    const auto fg_hi = static_cast<uint8_t>(fg_color >> 8);
    const auto fg_lo = static_cast<uint8_t>(fg_color & 0xFF);

    for (uint8_t row = 0; row < k_font_h; ++row)
    {
        const uint8_t row_bits = k_font8x8[cidx][row];
        for (uint8_t col = 0; col < k_font_w; ++col)
        {
            // Only write lit (foreground) pixels; background stays black from the zeroed row buffer.
            if (((row_bits >> col) & 1) == 0)
            {
                continue;
            }

            for (uint8_t sy = 0; sy < k_scale_y; ++sy)
            {
                for (uint8_t sx = 0; sx < k_scale_x; ++sx)
                {
                    const uint16_t px    = x + (col * k_scale_x) + sx;
                    const uint8_t  py    = (row * k_scale_y) + sy;
                    const size_t   i     = ((static_cast<size_t>(py) * k_width) + px) * 2;
                    m_rowbuf[i]     = fg_hi;
                    m_rowbuf[i + 1] = fg_lo;
                }
            }
        }
    }
}

void ST7789DisplayDevice::flushRow()
{
    setAddressWindow(0, m_cursor_y, k_width - 1, m_cursor_y + k_char_h - 1);
    m_dc.write(true);
    for (size_t offset = 0; offset < k_rowbuf_size; offset += k_spi_chunk)
    {
        const size_t chunk = ((k_rowbuf_size - offset) < k_spi_chunk)
                             ? (k_rowbuf_size - offset)
                             : k_spi_chunk;
        m_spi.transfer(m_rowbuf.data() + offset, nullptr, chunk);
    }
    std::fill(m_rowbuf.begin(), m_rowbuf.end(), 0);
}

// Resets cursor and zeroes the row buffer. Does NOT send a clear command to the hardware;
// old display content is overwritten as new rows are flushed during the next render pass.
void ST7789DisplayDevice::clear()
{
    std::fill(m_rowbuf.begin(), m_rowbuf.end(), 0);
    m_cursor_x = 0;
    m_cursor_y = 0;
}

// Sends zero-filled rows from the current cursor position to the bottom of the screen,
// blanking any content left over from a previous render that had more rows.
void ST7789DisplayDevice::flush()
{
    for (uint16_t y = m_cursor_y; y + k_char_h <= k_height; y += k_char_h)
    {
        setAddressWindow(0, y, k_width - 1, y + k_char_h - 1);
        m_dc.write(true);
        for (size_t offset = 0; offset < k_rowbuf_size; offset += k_spi_chunk)
        {
            const size_t chunk = ((k_rowbuf_size - offset) < k_spi_chunk)
                                 ? (k_rowbuf_size - offset)
                                 : k_spi_chunk;
            m_spi.transfer(m_rowbuf.data() + offset, nullptr, chunk);
        }
    }
}

void ST7789DisplayDevice::print(const char* text)
{
    printStyled(text, DisplayStyle::Normal);
}

void ST7789DisplayDevice::printStyled(const char* text, DisplayStyle style)
{
    const uint16_t color = styleToColor(style);

    for (const char* p = text; p != nullptr && *p != '\0'; ++p)
    {
        const char c = *p;
        // Skip non-ASCII bytes (e.g., UTF-8 multi-byte sequences like ─ and °)
        if (static_cast<uint8_t>(c) > k_display_on)
        {
            continue;
        }

        if (m_cursor_x + k_char_w > k_width)
        {
            m_cursor_x = 0;
            m_cursor_y += k_char_h;
        }

        if (m_cursor_y + k_char_h > k_height)
        {
            return;
        }

        drawChar(m_cursor_x, c, color);
        m_cursor_x += k_char_w;
    }

    // Flush the completed row, then advance to next line
    flushRow();
    m_cursor_x = 0;
    m_cursor_y += k_char_h;
}

uint16_t ST7789DisplayDevice::styleToColor(DisplayStyle style) noexcept
{
    switch (style)
    {
    case DisplayStyle::Red:
        return k_color_red;
    case DisplayStyle::Yellow:
        return k_color_yellow;
    case DisplayStyle::Green:
        return k_color_green;
    case DisplayStyle::Cyan:
        return k_color_cyan;
    case DisplayStyle::Dim:
        return k_color_blue;
    case DisplayStyle::Bold:   // intentional fall-through
    case DisplayStyle::Normal: // intentional fall-through
    default:
        return k_color_white;
    }
}

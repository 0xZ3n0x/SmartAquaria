#include "device/display/st7789/ST7789DisplayDevice.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "IClock.h"
#include "IGPIO.h"
#include "ISPI.h"
#include "device/display/IDisplayDevice.h"

using testing::_;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class MockClock : public IClock
{
  public:
    MOCK_METHOD(uint64_t, now_ms, (), (const, noexcept, override));
    MOCK_METHOD(void, sleep_ms, (uint32_t ms), (const, override));
};

class MockSPI : public ISPI
{
  public:
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, transfer, (const uint8_t* tx, uint8_t* rx, size_t len), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
};

class MockGPIO : public IGPIO
{
  public:
    MOCK_METHOD(bool, write, (bool active), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, close, (), (override));
};

// Captures pixel bytes sent to SPI during fn().
// Tracks the RAMWR command (0x2C, sent in command mode / DC low) as the
// start-of-pixel-data sentinel.  After RAMWR any DC-high transfer is pixel
// data.  A non-RAMWR command resets the sentinel so address-window coordinate
// bytes (sent between CASET/RASET and RAMWR) are not captured.
static std::vector<uint8_t> capturePixelsSent(MockSPI& spi, MockGPIO& dc,
                                               std::function<void()> fn)
{
    bool                 dc_high    = false;
    bool                 past_ramwr = false;
    std::vector<uint8_t> pixels;

    constexpr uint8_t k_cmd_ramwr = 0x2C;

    ON_CALL(dc, write(_))
        .WillByDefault(
            [&dc_high](bool active)
            {
                dc_high = active;
                return true;
            });
    ON_CALL(spi, transfer(_, _, _))
        .WillByDefault(
            [&dc_high, &past_ramwr, &pixels](const uint8_t* tx, uint8_t* /*rx*/, size_t len)
            {
                if (!dc_high)
                {
                    past_ramwr = (len == 1 && tx[0] == k_cmd_ramwr);
                }
                else if (past_ramwr)
                {
                    pixels.insert(pixels.end(), tx, tx + len);
                }
                return true;
            });

    fn();
    return pixels;
}

// ---------------------------------------------------------------------------
// Constructor test (standalone — expectations must be set before construction)
// ---------------------------------------------------------------------------

TEST(ST7789ConstructorTest, HardwareResetPinTogglesInCorrectSequence)
{
    NiceMock<MockSPI>   spi;
    NiceMock<MockGPIO>  dc;
    NiceMock<MockGPIO>  rst;
    NiceMock<MockClock> clock;

    ON_CALL(spi, transfer(_, _, _)).WillByDefault(Return(true));
    ON_CALL(dc, write(_)).WillByDefault(Return(true));
    ON_CALL(rst, write(_)).WillByDefault(Return(true));

    {
        InSequence seq;
        EXPECT_CALL(rst, write(true)).Times(1);
        EXPECT_CALL(rst, write(false)).Times(1);
        EXPECT_CALL(rst, write(true)).Times(1);
    }

    ST7789DisplayDevice display(spi, dc, rst, clock);
}

// ---------------------------------------------------------------------------
// Fixture — device constructed once per test in SetUp()
// ---------------------------------------------------------------------------

class ST7789DisplayDeviceTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        ON_CALL(m_spi, transfer(_, _, _)).WillByDefault(Return(true));
        ON_CALL(m_dc, write(_)).WillByDefault(Return(true));
        ON_CALL(m_rst, write(_)).WillByDefault(Return(true));

        m_display = std::make_unique<ST7789DisplayDevice>(m_spi, m_dc, m_rst, m_clock);
    }

    NiceMock<MockClock>                  m_clock;
    NiceMock<MockSPI>                    m_spi;
    NiceMock<MockGPIO>                   m_dc;
    NiceMock<MockGPIO>                   m_rst;
    std::unique_ptr<ST7789DisplayDevice> m_display;
};

TEST_F(ST7789DisplayDeviceTest, FlushClearsRowsBelowCursor)
{
    // After clear(), cursor is at (0,0); flush() must zero all 15 rows
    m_display->clear();
    const auto pixels = capturePixelsSent(m_spi, m_dc, [this] { m_display->flush(); });

    ASSERT_EQ(pixels.size(), static_cast<size_t>(240 * 240 * 2));
    const bool all_zero = std::all_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b == 0; });
    EXPECT_TRUE(all_zero);
}

TEST_F(ST7789DisplayDeviceTest, PrintProducesNonZeroPixels)
{
    const auto pixels = capturePixelsSent(m_spi, m_dc, [this] { m_display->print("A"); });

    const bool has_non_zero = std::any_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b != 0; });
    EXPECT_TRUE(has_non_zero);
}

TEST_F(ST7789DisplayDeviceTest, PrintStyledNormalProducesWhitePixels)
{
    const auto pixels =
        capturePixelsSent(m_spi, m_dc, [this] { m_display->printStyled("X", DisplayStyle::Normal); });

    // White in RGB565: 0xFFFF — both bytes are 0xFF
    const bool has_white = std::any_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b == 0xFF; });
    EXPECT_TRUE(has_white);
}

TEST_F(ST7789DisplayDeviceTest, PrintStyledRedProducesRedColorPixels)
{
    const auto pixels =
        capturePixelsSent(m_spi, m_dc, [this] { m_display->printStyled("X", DisplayStyle::Red); });

    // Red in RGB565: 0xF800 — high byte is 0xF8, low byte is 0x00
    const bool has_red = std::any_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b == 0xF8; });
    EXPECT_TRUE(has_red);
}

TEST_F(ST7789DisplayDeviceTest, PrintStyledYellowProducesYellowColorPixels)
{
    const auto pixels =
        capturePixelsSent(m_spi, m_dc, [this] { m_display->printStyled("X", DisplayStyle::Yellow); });

    // Yellow in RGB565: 0xFFE0 — high byte is 0xFF, low byte is 0xE0
    const bool has_yellow = std::any_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b == 0xE0; });
    EXPECT_TRUE(has_yellow);
}

TEST_F(ST7789DisplayDeviceTest, PrintStyledIgnoresCharactersAbove0x7E)
{
    // UTF-8 encoding of '°' (U+00B0): bytes 0xC2 and 0xB0 — both > 0x7E, must be skipped
    const auto pixels = capturePixelsSent(
        m_spi, m_dc, [this] { m_display->printStyled("\xC2\xB0", DisplayStyle::Normal); });

    // No glyph drawn — row sent with all black pixels
    const bool all_zero = std::all_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b == 0; });
    EXPECT_TRUE(all_zero);
}

TEST_F(ST7789DisplayDeviceTest, PrintDoesNotDrawBeyondScreenHeight)
{
    // k_char_h = 16px; 15 lines × 16px = 240px = k_height — fills the screen exactly
    for (int i = 0; i < 15; ++i)
        m_display->print("A");

    // 16th print: cursor_y (240) + k_char_h (16) > k_height (240) → early return, no draw
    const auto pixels = capturePixelsSent(m_spi, m_dc, [this] { m_display->print("B"); });
    EXPECT_TRUE(pixels.empty());
}

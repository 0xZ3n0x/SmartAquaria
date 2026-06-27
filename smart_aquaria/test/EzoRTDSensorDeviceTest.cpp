#include <memory>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "IUART.h"

#include "device/sensor/temp/EzoRTDSensorDevice.h"

class MockUART : public IUART
{
  public:
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(int, write, (const char* data, size_t len), (override));
    MOCK_METHOD(int, read, (char* buffer, size_t len), (override));
    MOCK_METHOD(void, flush, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
};

TEST(EzoRTDSensorDeviceTest, ReturnsNulloptWhenUARTClosed)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(false));

    EzoRTDSensorDevice device(mock);
    EXPECT_FALSE(device.getTemperature().has_value());
}

TEST(EzoRTDSensorDeviceTest, ReturnsNulloptWhenWriteFails)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::_, testing::_)).WillOnce(testing::Return(0));

    EzoRTDSensorDevice device(mock);
    EXPECT_FALSE(device.getTemperature().has_value());
}

TEST(EzoRTDSensorDeviceTest, ReturnsNulloptWhenReadReturnsNothing)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::_, testing::_)).WillOnce(testing::Return(2));
    EXPECT_CALL(mock, read(testing::_, testing::_)).WillOnce(testing::Return(0));

    EzoRTDSensorDevice device(mock);
    EXPECT_FALSE(device.getTemperature().has_value());
}

TEST(EzoRTDSensorDeviceTest, SendsCorrectCommand)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::StrEq("R\n"), 2U)).WillOnce(testing::Return(2));
    EXPECT_CALL(mock, read(testing::_, testing::_)).WillOnce(testing::Return(0));

    EzoRTDSensorDevice device(mock);
    device.getTemperature();
}

TEST(EzoRTDSensorDeviceTest, ParsesTemperatureFromResponse)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::_, testing::_)).WillOnce(testing::Return(2));
    EXPECT_CALL(mock, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "25.36\n";
            if (*pos >= response.size()) return 0;
            buf[0] = response[(*pos)++];
            return 1;
        });

    EzoRTDSensorDevice device(mock);
    const auto result = device.getTemperature();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(*result, 25.36F);
}

TEST(EzoRTDSensorDeviceTest, ReturnsNulloptOnInvalidResponse)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::_, testing::_)).WillOnce(testing::Return(2));
    EXPECT_CALL(mock, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "ERR";
            if (*pos >= response.size()) return 0;
            buf[0] = response[(*pos)++];
            return 1;
        });

    EzoRTDSensorDevice device(mock);
    EXPECT_FALSE(device.getTemperature().has_value());
}

TEST(EzoRTDSensorDeviceTest, ParsesNegativeTemperature)
{
    MockUART mock;
    EXPECT_CALL(mock, isOpen()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, write(testing::_, testing::_)).WillOnce(testing::Return(2));
    EXPECT_CALL(mock, read(testing::_, testing::_))
        .WillRepeatedly([pos = std::make_shared<size_t>(0)](char* buf, size_t) -> int {
            const std::string_view response = "-5.00\n";
            if (*pos >= response.size()) return 0;
            buf[0] = response[(*pos)++];
            return 1;
        });

    EzoRTDSensorDevice device(mock);
    const auto result = device.getTemperature();
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(*result, -5.00F);
}

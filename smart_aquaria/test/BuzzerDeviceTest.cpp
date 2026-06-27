#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "IGPIO.h"

#include "device/actuator/buzzer/BuzzerDevice.h"

class MockGPIO : public IGPIO
{
  public:
    MOCK_METHOD(bool, write, (bool active), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, close, (), (override));
};

TEST(BuzzerDeviceTest, BuzzOnWritesTrueToGPIO)
{
    MockGPIO mock;
    EXPECT_CALL(mock, write(true)).WillOnce(testing::Return(true));

    BuzzerDevice device(mock);
    EXPECT_TRUE(device.buzz(true));
}

TEST(BuzzerDeviceTest, BuzzOffWritesFalseToGPIO)
{
    MockGPIO mock;
    EXPECT_CALL(mock, write(false)).WillOnce(testing::Return(true));

    BuzzerDevice device(mock);
    EXPECT_TRUE(device.buzz(false));
}

TEST(BuzzerDeviceTest, BuzzReturnsFalseWhenWriteFails)
{
    MockGPIO mock;
    EXPECT_CALL(mock, write(testing::_)).WillOnce(testing::Return(false));

    BuzzerDevice device(mock);
    EXPECT_FALSE(device.buzz(true));
}

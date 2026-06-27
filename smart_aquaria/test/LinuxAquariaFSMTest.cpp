
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include "Clock.h"
#include "EventQueue.h"
#include "ILogger.h"
#include "app/SmartAquaria.h"
#include "domain/IActuator.h"
#include "domain/ISensor.h"
#include "domain/Thresholds.hpp"

// ---------------------------------------------------------------------------
// MockSensor / MockActuator / MockLogger
// ---------------------------------------------------------------------------

class MockSensor : public ISensor
{
  public:
    MOCK_METHOD(std::optional<float>, getTemperature, (), (override));
    MOCK_METHOD(std::optional<float>, getDO, (), (override));
};

class MockActuator : public IActuator
{
  public:
    MOCK_METHOD(bool, buzz, (bool on), (override));
    MOCK_METHOD(bool, heat, (bool on), (override));
    MOCK_METHOD(void, feed, (), (override));
};

class MockLogger : public ILogger
{
  public:
    MOCK_METHOD(bool, log, (const char* prefix, const char* message), (override));
};

// ---------------------------------------------------------------------------
// FSM test fixture
// ---------------------------------------------------------------------------

class AquariaFSMTest : public ::testing::Test
{
  protected:
    static constexpr int K = SmartAquaria::kHysteresisUp;
    static constexpr int F = SmartAquaria::kFaultThreshold;

    void SetUp() override
    {
        EXPECT_CALL(actuator_, buzz(testing::_)).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(actuator_, heat(testing::_)).WillRepeatedly(testing::Return(true));
        EXPECT_CALL(actuator_, feed()).WillRepeatedly(testing::Return());

        fsm_ = std::make_unique<SmartAquaria>(sensor_, actuator_, thresholds_, queue_, clock_, shutdown_);
    }

    void setReadings(float temp, float doVal)
    {
        EXPECT_CALL(sensor_, getTemperature()).WillRepeatedly(testing::Return(std::optional<float>{temp}));
        EXPECT_CALL(sensor_, getDO()).WillRepeatedly(testing::Return(std::optional<float>{doVal}));
    }

    void setFailingReadings()
    {
        EXPECT_CALL(sensor_, getTemperature()).WillRepeatedly(testing::Return(std::nullopt));
        EXPECT_CALL(sensor_, getDO()).WillRepeatedly(testing::Return(std::nullopt));
    }

    void dispatchSample(int n = 1)
    {
        for (int i = 0; i < n; ++i)
        {
            fsm_->dispatch(SmartAquaria::Event::SampleTimerElapsed);
        }
    }

    MockSensor                    sensor_;
    MockActuator                  actuator_;
    Thresholds                    thresholds_{20, 30, 16, 34, 6.0F, 5.0F};
    Clock                         clock_;
    EventQueue                    queue_;
    std::atomic<bool>             shutdown_{false};
    std::unique_ptr<SmartAquaria> fsm_;
};

// ---------------------------------------------------------------------------
// Basic state tests
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, StartsInNormal)
{
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, NormalSampleWithGoodReadingsStaysNormal)
{
    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

// ---------------------------------------------------------------------------
// Hysteresis — worsening requires K consecutive samples
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, HysteresisStopsAtK_Minus1_Warning)
{
    setReadings(31, 7.0F);
    dispatchSample(K - 1);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, HysteresisTransitsAtK_Warning)
{
    setReadings(31, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Warning);
}

TEST_F(AquariaFSMTest, HysteresisStopsAtK_Minus1_Alarm)
{
    setReadings(35, 7.0F);
    dispatchSample(K - 1);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, HysteresisTransitsAtK_Alarm)
{
    setReadings(35, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Alarm);
}

TEST_F(AquariaFSMTest, HysteresisCounterResetsOnGoodReading)
{
    setReadings(31, 7.0F);
    dispatchSample(K - 1);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Normal);

    setReadings(25, 7.0F);
    dispatchSample();
    ASSERT_EQ(fsm_->currentState(), AquariaState::Normal);

    setReadings(31, 7.0F);
    dispatchSample(K - 1);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

// ---------------------------------------------------------------------------
// Immediate recovery (single good sample)
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, ImmediateRecoveryFromWarningToNormal)
{
    setReadings(31, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Warning);

    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, ImmediateRecoveryFromAlarmToNormal)
{
    setReadings(35, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Alarm);

    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, AlarmToWarningOnNonCriticalReading)
{
    setReadings(35, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Alarm);

    setReadings(31, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Warning);
}

// ---------------------------------------------------------------------------
// Normal ↔ Warning ↔ Alarm transitions
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, NormalToWarningOnHighTemp)
{
    setReadings(31, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Warning);
}

TEST_F(AquariaFSMTest, NormalToWarningOnLowDO)
{
    setReadings(25, 5.5F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Warning);
}

TEST_F(AquariaFSMTest, NormalDirectToAlarmOnCriticalTemp)
{
    setReadings(35, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Alarm);
}

TEST_F(AquariaFSMTest, NormalDirectToAlarmOnCriticalLowDO)
{
    setReadings(25, 4.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Alarm);
}

TEST_F(AquariaFSMTest, WarningBackToNormalWhenConditionsRestore)
{
    setReadings(31, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Warning);

    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, WarningToAlarmOnCriticalTemp)
{
    setReadings(31, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Warning);

    setReadings(35, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Alarm);
}

TEST_F(AquariaFSMTest, AlarmBackToNormalWhenConditionsRestore)
{
    setReadings(35, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Alarm);

    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

// ---------------------------------------------------------------------------
// Buzzer entry/exit actions
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, AlarmEntryTurnsBuzzerOn)
{
    EXPECT_CALL(actuator_, buzz(true)).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    setReadings(35, 7.0F);
    dispatchSample(K);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Alarm);
}

TEST_F(AquariaFSMTest, AlarmExitTurnsBuzzerOff)
{
    EXPECT_CALL(actuator_, buzz(false)).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    setReadings(35, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Alarm);

    setReadings(25, 7.0F);
    dispatchSample();
    ASSERT_EQ(fsm_->currentState(), AquariaState::Normal);
}

// ---------------------------------------------------------------------------
// Feed — triggered by time check inside handleSample (setFeedInterval(0))
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, FeedInNormalCallsFeedOnce)
{
    EXPECT_CALL(actuator_, feed()).Times(1);

    fsm_->setFeedInterval(0);
    setReadings(25, 7.0F);
    dispatchSample();
}

TEST_F(AquariaFSMTest, FeedIgnoredInWarning)
{
    EXPECT_CALL(actuator_, feed()).Times(0);

    setReadings(31, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Warning);

    fsm_->setFeedInterval(0);
    dispatchSample();
}

TEST_F(AquariaFSMTest, FeedIgnoredInAlarm)
{
    EXPECT_CALL(actuator_, feed()).Times(0);

    setReadings(35, 7.0F);
    dispatchSample(K);
    ASSERT_EQ(fsm_->currentState(), AquariaState::Alarm);

    fsm_->setFeedInterval(0);
    dispatchSample();
}

// ---------------------------------------------------------------------------
// SensorFault state
// ---------------------------------------------------------------------------

TEST_F(AquariaFSMTest, SingleFailedReadingStaysNormal)
{
    setFailingReadings();
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, SensorFaultAfterConsecutiveFailures)
{
    setFailingReadings();
    dispatchSample(F);
    EXPECT_EQ(fsm_->currentState(), AquariaState::SensorFault);
}

TEST_F(AquariaFSMTest, SensorFaultNotTriggeredBeforeThreshold)
{
    setFailingReadings();
    dispatchSample(F - 1);
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, SensorFaultRecoveryToNormal)
{
    setFailingReadings();
    dispatchSample(F);
    ASSERT_EQ(fsm_->currentState(), AquariaState::SensorFault);

    setReadings(25, 7.0F);
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, SensorFaultEntryTurnsBuzzerOn)
{
    EXPECT_CALL(actuator_, buzz(true)).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    setFailingReadings();
    dispatchSample(F);
    EXPECT_EQ(fsm_->currentState(), AquariaState::SensorFault);
}

TEST_F(AquariaFSMTest, SensorFaultEntryTurnsHeaterOff)
{
    EXPECT_CALL(actuator_, heat(false)).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    setFailingReadings();
    dispatchSample(F);
    EXPECT_EQ(fsm_->currentState(), AquariaState::SensorFault);
}

TEST_F(AquariaFSMTest, SensorFaultExitTurnsBuzzerOff)
{
    EXPECT_CALL(actuator_, buzz(false)).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(true));

    setFailingReadings();
    dispatchSample(F);
    ASSERT_EQ(fsm_->currentState(), AquariaState::SensorFault);

    setReadings(25, 7.0F);
    dispatchSample();
    ASSERT_EQ(fsm_->currentState(), AquariaState::Normal);
}

TEST_F(AquariaFSMTest, SensorTimeoutInNormalDoesNotCrash)
{
    setFailingReadings();
    dispatchSample();
    EXPECT_EQ(fsm_->currentState(), AquariaState::Normal);
}

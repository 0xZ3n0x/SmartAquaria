#pragma once

#include <cstdint>
#include <optional>

struct Readings
{
    std::optional<float> temperature;    // nullopt when the last temperature read failed
    std::optional<float> do_value;       // nullopt when the last DO read failed
    bool     buzzer_on{false};
    bool     heater_on{false};
    uint32_t time_until_feed_s{0};       // seconds remaining until the next scheduled feed
};

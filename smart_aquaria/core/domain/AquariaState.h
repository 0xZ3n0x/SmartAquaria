#pragma once

// SensorFault is a non-operational state: heater is disabled and buzzer fires until a good read arrives.
enum class AquariaState { Normal, Warning, Alarm, SensorFault };
[[nodiscard]] const char* stateToString(AquariaState state) noexcept;
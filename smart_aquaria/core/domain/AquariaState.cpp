#include "domain/AquariaState.h"

const char* stateToString(AquariaState state) noexcept
{
    switch (state)
    {
        case AquariaState::Normal:      return "Normal";
        case AquariaState::Warning:     return "Warning";
        case AquariaState::Alarm:       return "Alarm";
        case AquariaState::SensorFault: return "SensorFault";
        default:                        return "Unknown";
    }
}
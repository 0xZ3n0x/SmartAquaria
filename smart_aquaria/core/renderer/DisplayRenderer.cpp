#include <array>
#include <cstdio>

#include "DisplayRenderer.h"
#include "device/display/IDisplayDevice.h"
#include "domain/AquariaState.h"
#include "domain/Readings.h"

namespace
{

// Single shared 32-byte buffer. Safe only because render() is called from one thread
// and each return value is consumed (passed to print/printStyled) before the next call.
template<typename... Args>
const char* fmt(const char* format, Args... args)
{
    static std::array<char, 32> buf{};
    std::snprintf(buf.data(), buf.size(), format, args...);
    return buf.data();
}

} // namespace

void DisplayRenderer::render(AquariaState state, const Readings& readings, IDisplayDevice& device) const
{
    device.clear();

    device.print("SmartAquaria");
    device.printStyled(stateToString(state), toStyle(state));
    device.print("");

    if (readings.temperature.has_value())
    {
        device.printStyled(fmt("Temp %.1f C", *readings.temperature), DisplayStyle::Cyan);
    }
        
    if (readings.do_value.has_value())
    {
        device.printStyled(fmt("DO %.2f mg/L", *readings.do_value), DisplayStyle::Cyan);
    }
        

    device.print("");

    device.printStyled(fmt("Feed %um", readings.time_until_feed_s / 60U), DisplayStyle::Bold);

    device.printStyled(fmt("Buzzer %s", readings.buzzer_on ? "ON" : "OFF"),
                       readings.buzzer_on ? DisplayStyle::Red : DisplayStyle::Dim);

    device.printStyled(fmt("Heater %s", readings.heater_on ? "ON" : "OFF"),
                       readings.heater_on ? DisplayStyle::Red : DisplayStyle::Dim);

    device.flush();
}

DisplayStyle DisplayRenderer::toStyle(AquariaState state)
{
    switch (state)
    {
    case AquariaState::Alarm:
        return DisplayStyle::Red;
    case AquariaState::Warning:
        return DisplayStyle::Yellow;
    case AquariaState::Normal:
    default:
        return DisplayStyle::Green;
    }
}

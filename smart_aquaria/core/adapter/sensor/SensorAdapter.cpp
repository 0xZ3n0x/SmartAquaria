#include "SensorAdapter.h"

#include "device/sensor/do/IDOSensor.h"
#include "device/sensor/temp/ITemperatureSensor.h"

SensorAdapter::SensorAdapter(ITemperatureSensor& tempDevice, IDOSensor& doDevice)
    : m_temp_device(tempDevice), m_do_device(doDevice)
{
}

std::optional<float> SensorAdapter::getTemperature()
{
    return m_temp_device.getTemperature();
}

std::optional<float> SensorAdapter::getDO()
{
    return m_do_device.getDO();
}

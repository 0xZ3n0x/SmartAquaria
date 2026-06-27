add_library(SmartAquaria_Core STATIC
    adapter/actuator/ActuatorAdapter.cpp
    adapter/display/DisplayAdapter.cpp
    adapter/sensor/SensorAdapter.cpp
    app/SmartAquaria.cpp
    device/actuator/buzzer/BuzzerDevice.cpp
    device/actuator/heater/HeaterDevice.cpp
    device/display/st7789/ST7789DisplayDevice.cpp
    device/sensor/do/EzoDOSensorDevice.cpp
    device/sensor/temp/EzoRTDSensorDevice.cpp
    domain/AquariaState.cpp
    renderer/DisplayRenderer.cpp
)

target_include_directories(SmartAquaria_Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SmartAquaria_Core PUBLIC
    SmartAquaria_OSAL_Interface
    SmartAquaria_HAL_Interface
)

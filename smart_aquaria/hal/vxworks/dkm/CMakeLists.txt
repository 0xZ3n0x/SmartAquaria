message(STATUS "HAL: VxWorks DKM")

add_library(SmartAquaria_HAL STATIC
    gpio/GPIO.cpp
    spi/SPI.cpp
    uart/UART.cpp
)

target_include_directories(SmartAquaria_HAL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio
    ${CMAKE_CURRENT_SOURCE_DIR}/spi
    ${CMAKE_CURRENT_SOURCE_DIR}/uart
)

target_link_libraries(SmartAquaria_HAL PUBLIC SmartAquaria_HAL_Interface)

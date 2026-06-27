add_library(SmartAquaria_Configurator_Common STATIC
    AppRunner.cpp
    TerminalDisplayDevice.cpp
)

target_include_directories(SmartAquaria_Configurator_Common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SmartAquaria_Configurator_Common PUBLIC
    SmartAquaria_Core
    SmartAquaria_OSAL
    SmartAquaria_HAL_Interface
    tomlplusplus::tomlplusplus
)

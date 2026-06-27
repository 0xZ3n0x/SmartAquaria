add_executable(smart_aquaria_linux main.cpp)

target_link_libraries(smart_aquaria_linux
    PRIVATE
        SmartAquaria_Configurator_Common
        SmartAquaria_HAL
)

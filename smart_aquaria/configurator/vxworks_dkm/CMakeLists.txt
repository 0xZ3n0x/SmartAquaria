add_executable(smart_aquaria_vxworks_dkm main.cpp)

target_link_libraries(smart_aquaria_vxworks_dkm
    PRIVATE
        SmartAquaria_Configurator_Common
        SmartAquaria_HAL
)

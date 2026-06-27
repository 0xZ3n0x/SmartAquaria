set(SMART_AQUARIA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(smart_aquaria_tests
    BuzzerDeviceTest.cpp
    EzoRTDSensorDeviceTest.cpp
    ST7789DisplayDeviceTest.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_sources(smart_aquaria_tests PRIVATE
        TerminalDisplayDeviceTest.cpp
        LinuxAquariaFSMTest.cpp
        LinuxLoggerTest.cpp
        LinuxSmartAquariaTest.cpp
    )

    target_link_libraries(smart_aquaria_tests PRIVATE
        SmartAquaria_Configurator_Common
        SmartAquaria_HAL
    )
endif()

target_link_libraries(smart_aquaria_tests
    PRIVATE
        SmartAquaria_Core
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(smart_aquaria_tests)

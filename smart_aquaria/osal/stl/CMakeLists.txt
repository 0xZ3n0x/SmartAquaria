add_library(SmartAquaria_OSAL STATIC
    clock/Clock.cpp
    event_queue/EventQueue.cpp
    logger/Logger.cpp
    timer/TimerService.cpp
)

target_include_directories(SmartAquaria_OSAL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/clock
    ${CMAKE_CURRENT_SOURCE_DIR}/event_queue
    ${CMAKE_CURRENT_SOURCE_DIR}/logger
    ${CMAKE_CURRENT_SOURCE_DIR}/timer
)

target_link_libraries(SmartAquaria_OSAL PUBLIC SmartAquaria_OSAL_Interface)

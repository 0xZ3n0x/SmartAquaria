add_subdirectory(interface)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    add_subdirectory(stl)
else()
    message(STATUS "OSAL: '${CMAKE_SYSTEM_NAME}' is not supported")
endif()

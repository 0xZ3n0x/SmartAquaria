add_subdirectory(interface)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    add_subdirectory(vxworks)
else()
    message(STATUS "HAL: '${CMAKE_SYSTEM_NAME}' is not supported")
endif()

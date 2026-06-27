cmake_minimum_required(VERSION 3.20)
project(SmartAquaria VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(osal)
add_subdirectory(hal)
add_subdirectory(core)
add_subdirectory(configurator)

if(CMAKE_SYSTEM_NAME MATCHES "Linux|Windows|Darwin")
    enable_testing()
    add_subdirectory(test)
endif()

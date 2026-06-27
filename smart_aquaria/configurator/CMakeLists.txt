include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

add_subdirectory(common)

if(CMAKE_SYSTEM_NAME MATCHES "Linux|Windows|Darwin")
    add_subdirectory(linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "VxWorks")
    add_subdirectory(vxworks_dkm)
endif()

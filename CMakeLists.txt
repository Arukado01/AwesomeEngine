cmake_minimum_required(VERSION 3.28)
project(AwesomeEngine CXX)

set(SFML_BUILD_NETWORK OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(sfml GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 3.1.0)
FetchContent_MakeAvailable(sfml)

file(GLOB SOURCES CONFIGURE_DEPENDS Engine/Source/*.cpp)
add_executable(AwesomeEngine ${SOURCES})

target_include_directories(AwesomeEngine PRIVATE Engine/Include)
target_compile_features(AwesomeEngine PRIVATE cxx_std_20)
target_compile_options(AwesomeEngine PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>, /W4, -Wall -Wextra>)
target_link_libraries(AwesomeEngine PRIVATE SFML::Graphics SFML::Audio)

cmake_minimum_required(VERSION 3.28)
project(AwesomeEngine CXX)

set(SFML_BUILD_NETWORK OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SPDLOG_USE_STD_FORMAT ON)
set(TGUI_BACKEND SFML_GRAPHICS)

include(FetchContent)
FetchContent_Declare(sfml GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 3.1.0)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_Declare(logs URL https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz)
FetchContent_Declare(tgui URL https://github.com/texus/TGUI/archive/refs/tags/v1.13.0.tar.gz)
FetchContent_MakeAvailable(sfml json logs tgui)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS Engine/Source/*.cpp)
add_executable(AwesomeEngine ${SOURCES})

target_include_directories(AwesomeEngine PRIVATE Engine/Include)
target_compile_features(AwesomeEngine PRIVATE cxx_std_20)
target_compile_options(AwesomeEngine PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>, /W4, -Wall -Wextra>)
target_link_libraries(AwesomeEngine PRIVATE SFML::Graphics SFML::Audio nlohmann_json::nlohmann_json spdlog::spdlog TGUI::TGUI)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group")
  set(CMAKE_CXX_STANDARD_LIBRARIES "-Wl,--end-group")
endif()

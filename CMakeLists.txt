cmake_minimum_required(VERSION 4.2)
project(vulkan_renderer)

set(CMAKE_CXX_STANDARD 20)

# Homebrew installs CMake package files outside CLion's default search paths.
find_package(nlohmann_json CONFIG REQUIRED
        PATHS
        /opt/homebrew/opt/nlohmann-json
        /usr/local/opt/nlohmann-json)

add_executable(vulkan_renderer main.cpp
        GLTFParser.cpp)

target_link_libraries(vulkan_renderer PRIVATE nlohmann_json::nlohmann_json)

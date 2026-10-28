cmake_minimum_required(VERSION 3.20)
project(ECOSStudio VERSION 0.4.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ecos_studio_core
    src/core.cpp
    src/project.cpp
    src/result.cpp
    src/platform.cpp
    src/scene.cpp
    src/asset_registry.cpp
)
target_include_directories(ecos_studio_core PUBLIC include)
target_compile_features(ecos_studio_core PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(ecos_studio_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(ecos_studio_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(ecos-studio src/main.cpp)
target_link_libraries(ecos-studio PRIVATE ecos_studio_core)
enable_testing()
add_executable(ecos-studio-core-tests tests/core_tests.cpp)
target_link_libraries(ecos-studio-core-tests PRIVATE ecos_studio_core)
add_test(NAME core_lifecycle_and_logging COMMAND ecos-studio-core-tests)
add_executable(ecos-studio-platform-tests tests/platform_tests.cpp)
target_link_libraries(ecos-studio-platform-tests PRIVATE ecos_studio_core)
add_test(NAME platform_paths_filesystem_and_clock COMMAND ecos-studio-platform-tests)
add_executable(ecos-studio-scene-asset-tests tests/scene_asset_tests.cpp)
target_link_libraries(ecos-studio-scene-asset-tests PRIVATE ecos_studio_core)
add_test(NAME scene_graph_and_asset_registry COMMAND ecos-studio-scene-asset-tests)
add_test(NAME cli_help COMMAND ecos-studio --help)

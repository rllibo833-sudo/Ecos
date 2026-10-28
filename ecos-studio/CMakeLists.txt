cmake_minimum_required(VERSION 3.20)
project(ECOSStudio VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ecos_studio_core src/core.cpp src/project.cpp)
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
add_test(NAME cli_help COMMAND ecos-studio --help)

cmake_minimum_required(VERSION 3.22.1)

project(Starfox64Recompiled LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_definitions(ANDROID SDL_MAIN_HANDLED)

# Find required libraries
find_library(log-lib log)
find_library(android-lib android)
find_library(z-lib z)

# Include SDL2 (will be fetched by root CMakeLists.txt if needed)
if(NOT TARGET SDL2::SDL2)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.3.zip
    )
    FetchContent_MakeAvailable(SDL2)
endif()

# Create a minimal native library
add_library(starfox64recompiled SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/cpp/SDL_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/cpp/main_android_wrapper.cpp
)

# Include directories
target_include_directories(starfox64recompiled PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)

# Link libraries
target_link_libraries(starfox64recompiled
    SDL2::SDL2
    ${log-lib}
    ${android-lib}
    ${z-lib}
)

# Set output directory
set_target_properties(starfox64recompiled PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
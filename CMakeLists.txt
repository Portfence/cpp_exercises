cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_COMPILER clang++-19)
set(CMAKE_C_COMPILER clang-19)

project(Exercises LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(IWYU_PATH NAMES include-what-you-use iwyu)
if(IWYU_PATH)
    message(STATUS "Found include-what-you-use: ${IWYU_PATH}")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})
else()
    message(WARNING "include-what-you-use not found")
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/echo.cpp
    src/video_tracker/app.cpp
    src/video_tracker/decoder.cpp
    src/video_tracker/viewer.cpp
    src/video_tracker/playback.cpp
)

# target_include_directories(${PROJECT_NAME} PRIVATE src/include)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wswitch -Os)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-Werror>
)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)
include(cmake/FFmpeg.cmake)

find_package(OpenGL REQUIRED)
find_package(CUDAToolkit REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)

glad_add_library(glad_gl STATIC
    REPRODUCIBLE
    API gl:core=3.3
    EXTENSIONS NONE
)


target_include_directories(${PROJECT_NAME} PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE ASIO_NO_DEPRECATED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    FFmpeg::avformat
    FFmpeg::avcodec
    FFmpeg::avutil
    glfw
    OpenGL::GL
    CUDA::cuda_driver
    glad_gl
)


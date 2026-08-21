cmake_minimum_required(VERSION 3.28.3)
project(Exercises LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/echo.cpp
)

# target_include_directories(${PROJECT_NAME} PRIVATE src/include)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Os)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-Werror>
)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2   # check GitHub for the latest release tag
)
FetchContent_MakeAvailable(asio) 
target_include_directories(${PROJECT_NAME} PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE ASIO_NO_DEPRECATED)

cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_COMPILER clang++-19)
set(CMAKE_C_COMPILER clang-19)

project(Exercises LANGUAGES CXX)

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
target_include_directories(${PROJECT_NAME} PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASIO_STANDALONE ASIO_NO_DEPRECATED)



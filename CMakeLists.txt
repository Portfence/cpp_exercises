cmake_minimum_required(VERSION 3.28.3)
project(Exercises LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

# target_include_directories(${PROJECT_NAME} PRIVATE src/include)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-Werror>
)
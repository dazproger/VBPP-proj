cmake_minimum_required(VERSION 3.27)
project(vector_bin_packing 
        VERSION 1.0
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(${PROJECT_NAME}
    project.cpp
    project.h)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:
    -O3
    -march=native
    -flto
    >)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
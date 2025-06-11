cmake_minimum_required(VERSION 3.10)
project(PhigrosAutoPlay VERSION 1.0)

file(GLOB_RECURSE SRC_FILES 
    "main.cpp"
    "Phigros/*.cpp"
    "Phigros/Touch/*.cpp"
)

add_executable(AutoPlay ${SRC_FILES})

target_include_directories(AutoPlay PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Phigros
)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -flto -fno-rtti")
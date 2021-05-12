cmake_minimum_required(VERSION 3.17)
project(LV6)

set(CMAKE_CXX_STANDARD 14)

add_executable(LV6 main.cpp functions.cpp functions.h)

find_package(OpenCV)
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(LV6 ${OpenCV_LIBS})
    message(STATUS "Opened OpenCV Successfully!")
endif(OpenCV_FOUND)
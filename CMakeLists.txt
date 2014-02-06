cmake_minimum_required (VERSION 2.8)
project (CL1)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(${PROJECT_SOURCE_DIR} /usr/include/nvidia-319-updates/)
link_directories(${PROJECT_BINARY_DIR})

add_executable(cl1 utils.cpp main.cpp)
target_link_libraries(cl1 OpenCL)

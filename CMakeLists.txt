cmake_minimum_required(VERSION 3.18.4)

set (CMAKE_CXX_STANDARD 20)

project(task)

add_executable(${PROJECT_NAME} main.cpp) 

add_subdirectory(event-handler)

target_link_libraries(${PROJECT_NAME} eventHandler) 

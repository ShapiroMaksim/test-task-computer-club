cmake_minimum_required(VERSION 3.18.4)

set (CMAKE_CXX_STANDARD 20)

project(eventHandler)

add_library(${PROJECT_NAME} eventHandler.cpp)

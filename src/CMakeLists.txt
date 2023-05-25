cmake_minimum_required(VERSION 3.26.1)	

set(PROJECT_NAME tcp-calc)
project(${PROJECT_NAME})

set(SOURCE_EXE
main.cpp
sockets.cpp
chat.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

cmake_minimum_required(VERSION 3.20)

#set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME tcp-calc)
project(${PROJECT_NAME})

set(SOURCE_EXE
main.cpp
sockets.cpp
chat.cpp
calc.cpp
dbase_and_log.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(/usr/local/include/)

find_package(SQLite3 REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE_EXE})
target_link_libraries (${PROJECT_NAME} SQLite::SQLite3)

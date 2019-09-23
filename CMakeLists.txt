cmake_minimum_required(VERSION 3.0)
project(ircpp CXX)

find_package(cpprestsdk REQUIRED)
set(CMAKE_CXX_STANDARD 17)

add_library(ircpp INTERFACE)

target_sources(ircpp INTERFACE ${PROJECT_SOURCE_DIR}/src/message.cpp ${PROJECT_SOURCE_DIR}/src/client.cpp)
target_include_directories(ircpp INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ircpp INTERFACE cpprestsdk::cpprest)

cmake_minimum_required(VERSION 3.0)
project(libircpp CXX)

find_library(CPPREST cpprest)
set(CMAKE_CXX_STANDARD 17)

add_library(libirc SHARED src/message.cpp src/irc_client.cpp)

target_include_directories(libirc PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(libirc ${CPPREST})
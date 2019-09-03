cmake_minimum_required(VERSION 3.0)
project(ircpp CXX)

find_library(CPPREST cpprest)
set(CMAKE_CXX_STANDARD 17)

add_library(ircpp SHARED src/message.cpp src/client.cpp)

target_include_directories(ircpp PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ircpp ${CPPREST})

add_executable(example_twitch example/twitch.cpp)
target_link_libraries(example_twitch ircpp)
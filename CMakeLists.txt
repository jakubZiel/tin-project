cmake_minimum_required(VERSION 3.21)
project(tin_21z)
include_directories(include)

set(CMAKE_CXX_STANDARD 14)

add_executable(tin-server
        server/Server.cpp
        )
add_executable(tin-client
        client/Client.cpp
        )

cmake_minimum_required(VERSION 3.21)
project(tin_21z)
include_directories(include)

set(CMAKE_CXX_STANDARD 14)

add_executable(server
        server/Server.cpp
        )
add_executable(tin-client
        client/Client.cpp
        )

add_executable(server-adm
            server/AdminServer.cpp
        )

add_executable(client-adm
        client/AdminClient.cpp
        )

add_executable(client_test
        client/test_c.cpp)

add_executable(client_test
        client/test_c.cpp)